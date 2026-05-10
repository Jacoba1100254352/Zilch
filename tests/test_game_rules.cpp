#include "Checker.h"
#include "Dice.h"
#include "GameManager.h"
#include "GameStateMachine.h"
#include "Player.h"
#include "ResourcePath.h"
#include "Score.h"
#include "rules/RuleManager.h"

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace {
void expect(const bool condition, const std::string& message)
{
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

template <typename Actual, typename Expected>
void expectEqual(const Actual& actual, const Expected& expected, const std::string& message)
{
    if (!(actual == expected)) {
        std::cerr << "FAIL: " << message << " (expected " << expected << ", got " << actual << ")\n";
        std::exit(EXIT_FAILURE);
    }
}

bool hasOption(const std::vector<Checker::Option>& options, const std::string& label)
{
    for (const Checker::Option& option : options)
        if (option.label == label)
            return true;
    return false;
}

void runOption(const std::vector<Checker::Option>& options, const std::string& label)
{
    for (const Checker::Option& option : options) {
        if (option.label == label) {
            option.action();
            return;
        }
    }
    expect(false, "missing option: " + label);
}

void prepareTurn(GameManager& game, const std::vector<std::string>& players)
{
    game.setPlayers(players);
    game.setScoreLimit(5000);
    game.startTurn(0);
}

Dice& setDice(GameManager& game, const std::initializer_list<std::pair<uint16_t, uint16_t>> values)
{
    Dice& dice = game.getCurrentPlayer()->getDice();
    dice.diceSetMap.clear();

    uint16_t count = 0;
    for (const auto& [dieValue, copies] : values) {
        if (copies == 0)
            continue;
        dice.diceSetMap[dieValue] = copies;
        count = static_cast<uint16_t>(count + copies);
    }

    dice.setNumDiceInPlay(count);
    dice.calculateMultipleAvailability();
    return dice;
}

void runTest(const std::string& name, const std::function<void()>& test)
{
    test();
    std::cout << "PASS: " << name << '\n';
}

void diceRollMaintainsConsistentState()
{
    Dice dice;
    dice.setNumDiceInPlay(99);
    expectEqual(dice.getNumDiceInPlay(), static_cast<uint16_t>(FULL_SET_OF_DICE), "dice count should clamp to full set");

    dice.setNumDiceInPlay(4);
    dice.rollDice();

    expectEqual(dice.getLastRoll().size(), static_cast<std::size_t>(4), "last roll should contain one value per active die");

    uint16_t countedDice = 0;
    bool hasMultiple = false;
    for (const uint16_t value : dice.getLastRoll()) {
        expect(value >= 1 && value <= FULL_SET_OF_DICE, "rolled die value should stay in the valid range");
    }
    for (const auto& [dieValue, copies] : dice.diceSetMap) {
        expect(dieValue >= 1 && dieValue <= FULL_SET_OF_DICE, "dice map key should stay in the valid range");
        countedDice = static_cast<uint16_t>(countedDice + copies);
        hasMultiple = hasMultiple || copies >= 3;
    }

    expectEqual(countedDice, dice.getNumDiceInPlay(), "dice map counts should match the active dice count");
    expectEqual(dice.isMultipleAvailable(), hasMultiple, "multiple flag should match dice map contents");

    dice.diceSetMap = {{2, 3}, {5, 1}};
    dice.calculateMultipleAvailability();
    expect(dice.isMultipleAvailable(), "triple should set multiple availability");
    dice.eliminateDice(2);
    dice.calculateMultipleAvailability();
    expect(!dice.isMultipleAvailable(), "eliminating the triple should clear multiple availability");
    expect(dice.diceSetMap.count(2) == 0, "eliminating dice should remove the value from the map");
}

void scoreAndPlayerDefaultsAreStable()
{
    Score score;
    expectEqual(score.getScoreLimit(), static_cast<uint32_t>(2000), "score should default to the original game limit");
    expectEqual(score.getPermanentScore(), static_cast<uint32_t>(0), "permanent score should default to zero");
    expectEqual(score.getRoundScore(), static_cast<uint32_t>(0), "round score should default to zero");
    expectEqual(score.getScoreFromMultiples(), static_cast<uint32_t>(0), "saved multiple score should default to zero");

    score.setScoreLimit(5000);
    score.increasePermanentScore(1250);
    score.increaseRoundScore(350);
    score.setScoreFromMultiples(200);
    expectEqual(score.getScoreLimit(), static_cast<uint32_t>(5000), "score limit setter should persist");
    expectEqual(score.getPermanentScore(), static_cast<uint32_t>(1250), "permanent score increments should persist");
    expectEqual(score.getRoundScore(), static_cast<uint32_t>(350), "round score increments should persist");
    expectEqual(score.getScoreFromMultiples(), static_cast<uint32_t>(200), "saved multiple score setter should persist");

    Player player("Alice");
    expectEqual(player.getName(), std::string("Alice"), "player should retain its configured name");
    expectEqual(player.getDice().getNumDiceInPlay(), static_cast<uint16_t>(FULL_SET_OF_DICE), "new players should start with a full dice set");
    expectEqual(player.getScore().getPermanentScore(), static_cast<uint32_t>(0), "new players should start unscored");
}

void gameStateMachineDescribesEveryState()
{
    GameStateMachine stateMachine;
    expectEqual(stateMachine.describe(), std::string("Intro"), "state machine should default to Intro");

    const std::vector<std::pair<GameStateMachine::State, std::string>> states = {
        {GameStateMachine::State::Intro, "Intro"},
        {GameStateMachine::State::Rules, "Rules"},
        {GameStateMachine::State::Setup, "Setup"},
        {GameStateMachine::State::PlayerTurn, "PlayerTurn"},
        {GameStateMachine::State::Rolling, "Rolling"},
        {GameStateMachine::State::Resolving, "Resolving"},
        {GameStateMachine::State::Switching, "Switching"},
        {GameStateMachine::State::GameOver, "GameOver"},
    };

    for (const auto& [state, description] : states) {
        stateMachine.setState(state);
        expect(stateMachine.getState() == state, "state machine should retain assigned state");
        expectEqual(stateMachine.describe(), description, "state machine should describe every declared state");
    }
}

void singlesDoNotMasqueradeAsMultiples()
{
    GameManager game;
    prepareTurn(game, {"A", "B"});

    Dice& dice = setDice(game, {{1, 3}, {5, 1}});

    const Checker checker(game);
    const std::vector<Checker::Option> options = checker.getAvailableOptions();

    expect(hasOption(options, "Score multiples of 1"), "triple ones should offer a multiple option");
    expect(!hasOption(options, "Score single 1 (100)"), "triple ones should not expose a misleading single-one option");
    expect(hasOption(options, "Score single 5 (50)"), "standalone five should remain scoreable");

    runOption(options, "Score single 5 (50)");
    expect(game.getCurrentPlayer()->getScore().getRoundScore() == 50, "single five should score exactly 50");
    expect(dice.getNumDiceInPlay() == 3, "single five should remove one die");
    expect(dice.diceSetMap.count(5) == 0, "single five should remove only the five");
    expect(dice.diceSetMap.at(1) == 3, "single five should leave the triple ones intact");
}

void singlesScoreOnlyOneDie()
{
    GameManager game;
    prepareTurn(game, {"A", "B"});

    Dice& dice = setDice(game, {{1, 2}});

    const Checker checker(game);
    const std::vector<Checker::Option> options = checker.getAvailableOptions();
    runOption(options, "Score single 1 (100)");

    expect(game.getCurrentPlayer()->getScore().getRoundScore() == 100, "single one should score exactly 100");
    expect(dice.getNumDiceInPlay() == 1, "single one should remove exactly one die");
    expect(dice.diceSetMap.at(1) == 1, "one die of value one should remain");
}

void straightsAndSetsScoreHotDice()
{
    GameManager straightGame;
    prepareTurn(straightGame, {"A", "B"});
    Dice& straightDice = setDice(straightGame, {{1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}, {6, 1}});

    const Checker straightChecker(straightGame);
    std::vector<Checker::Option> options = straightChecker.getAvailableOptions();
    expect(hasOption(options, "Apply strait (1000)"), "one-through-six should expose straight scoring");
    runOption(options, "Apply strait (1000)");

    expectEqual(straightGame.getCurrentPlayer()->getScore().getRoundScore(), static_cast<uint32_t>(1000), "straight should score 1000");
    expectEqual(straightDice.getNumDiceInPlay(), static_cast<uint16_t>(FULL_SET_OF_DICE), "scoring all dice should reset to hot dice");
    expect(straightDice.diceSetMap.empty(), "straight should consume all displayed dice");
    expect(straightGame.getSelectedOptionStatus(), "straight should mark an option as selected");

    GameManager setGame;
    prepareTurn(setGame, {"A", "B"});
    Dice& setDiceRef = setDice(setGame, {{1, 2}, {3, 2}, {6, 2}});

    const Checker setChecker(setGame);
    options = setChecker.getAvailableOptions();
    expect(hasOption(options, "Apply set (3 pairs)"), "three pairs should expose set scoring");
    runOption(options, "Apply set (3 pairs)");

    expectEqual(setGame.getCurrentPlayer()->getScore().getRoundScore(), static_cast<uint32_t>(1000), "three pairs should score 1000");
    expectEqual(setDiceRef.getNumDiceInPlay(), static_cast<uint16_t>(FULL_SET_OF_DICE), "set should reset to hot dice");
    expect(setDiceRef.diceSetMap.empty(), "set should consume all displayed dice");
}

void multiplesScoreScaleAndCanBeExtended()
{
    GameManager game;
    prepareTurn(game, {"A", "B"});
    Dice& dice = setDice(game, {{2, 3}, {5, 1}});

    const Checker checker(game);
    std::vector<Checker::Option> options = checker.getAvailableOptions();
    expect(hasOption(options, "Score multiples of 2"), "triple twos should expose multiple scoring");
    runOption(options, "Score multiples of 2");

    Score& score = game.getCurrentPlayer()->getScore();
    expectEqual(score.getRoundScore(), static_cast<uint32_t>(200), "triple twos should score 200");
    expectEqual(score.getScoreFromMultiples(), static_cast<uint16_t>(200), "saved multiple score should track the triple");
    expectEqual(game.getValueOfChosenMultiple(), static_cast<uint16_t>(2), "chosen multiple value should be saved");
    expectEqual(dice.getNumDiceInPlay(), static_cast<uint16_t>(1), "triple twos should leave the non-scoring die");
    expect(dice.diceSetMap.count(2) == 0, "multiple scoring should remove all scored twos");
    expectEqual(dice.diceSetMap.at(5), static_cast<uint16_t>(1), "multiple scoring should leave unrelated dice");

    setDice(game, {{2, 1}, {5, 1}});
    options = checker.getAvailableOptions();
    expect(hasOption(options, "Score multiples of 2"), "saved multiple should be extendable on a later roll");
    runOption(options, "Score multiples of 2");

    expectEqual(score.getRoundScore(), static_cast<uint32_t>(400), "adding one more two should double the saved multiple score");
    expectEqual(score.getScoreFromMultiples(), static_cast<uint16_t>(400), "saved multiple score should update after extension");
    expectEqual(dice.getNumDiceInPlay(), static_cast<uint16_t>(1), "extension should remove only the matching die");
    expect(dice.diceSetMap.count(2) == 0, "extension should remove the matching die from the map");
}

void fourOnesDoubleBaseMultipleAndHotDice()
{
    GameManager game;
    prepareTurn(game, {"A", "B"});
    Dice& dice = setDice(game, {{1, 4}});

    const Checker checker(game);
    const std::vector<Checker::Option> options = checker.getAvailableOptions();
    runOption(options, "Score multiples of 1");

    const Score& score = game.getCurrentPlayer()->getScore();
    expectEqual(score.getRoundScore(), static_cast<uint32_t>(2000), "four ones should double the 1000-point base");
    expectEqual(score.getScoreFromMultiples(), static_cast<uint16_t>(2000), "saved multiple should retain the scaled one score");
    expectEqual(dice.getNumDiceInPlay(), static_cast<uint16_t>(FULL_SET_OF_DICE), "scoring every die should reset to hot dice");
    expect(dice.diceSetMap.empty(), "all ones should be consumed");
}

void ruleManagerExposesStructuredGameOptions()
{
    GameManager game;
    prepareTurn(game, {"A", "B"});
    setDice(game, {{2, 3}, {5, 1}});

    zilch::rules::RuleManager ruleManager(game.getRuleConfig());
    const std::vector<zilch::rules::GameOption> options = ruleManager.evaluate(game);

    zilch::rules::GameOption multipleOption{};
    bool found = false;
    for (const zilch::rules::GameOption& option : options) {
        if (option.ruleType == zilch::rules::RuleType::Multiple && option.selectedValue == 2) {
            multipleOption = option;
            found = true;
            break;
        }
    }

    expect(found, "rule manager should expose a structured multiple option");
    expectEqual(multipleOption.label, std::string("Score multiples of 2"), "structured option should preserve UI label");
    expectEqual(multipleOption.pointsAwarded, static_cast<uint32_t>(200), "structured option should expose awarded points");
    expectEqual(multipleOption.accumulatedMultipleScore, static_cast<uint32_t>(200), "structured option should expose saved multiple total");
    expectEqual(multipleOption.consumedDice.at(2), static_cast<uint16_t>(3), "structured option should expose consumed dice");

    ruleManager.apply(game, multipleOption);

    expectEqual(game.getCurrentPlayer()->getScore().getRoundScore(), static_cast<uint32_t>(200), "rule manager should apply score through structured option");
    expectEqual(game.getValueOfChosenMultiple(), static_cast<uint16_t>(2), "rule manager should update saved multiple value");
    expectEqual(game.getCurrentPlayer()->getDice().getNumDiceInPlay(), static_cast<uint16_t>(1), "rule manager should consume scored dice");
}

void ruleTogglesSuppressOnlyTheirConfiguredOptions()
{
    GameManager straightGame;
    prepareTurn(straightGame, {"A", "B"});
    straightGame.accessRuleConfig().setStraitsEnabled(false);
    setDice(straightGame, {{1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}, {6, 1}});
    const Checker straightChecker(straightGame);
    std::vector<Checker::Option> options = straightChecker.getAvailableOptions();
    expect(!hasOption(options, "Apply strait (1000)"), "disabled straights should not be offered");
    expect(hasOption(options, "Score single 1 (100)"), "disabled straights should not suppress singles");
    expect(hasOption(options, "Score single 5 (50)"), "disabled straights should not suppress singles");

    GameManager setGame;
    prepareTurn(setGame, {"A", "B"});
    setGame.accessRuleConfig().setSetsEnabled(false);
    setDice(setGame, {{1, 2}, {3, 2}, {6, 2}});
    const Checker setChecker(setGame);
    expect(!hasOption(setChecker.getAvailableOptions(), "Apply set (3 pairs)"), "disabled sets should not be offered");

    GameManager multipleGame;
    prepareTurn(multipleGame, {"A", "B"});
    multipleGame.accessRuleConfig().setMultiplesEnabled(false);
    setDice(multipleGame, {{2, 3}});
    const Checker multipleChecker(multipleGame);
    expect(!hasOption(multipleChecker.getAvailableOptions(), "Score multiples of 2"), "disabled multiples should not be offered");
    expect(!multipleChecker.isOptionAvailable(), "triple twos should have no options when multiples are disabled");

    GameManager singleGame;
    prepareTurn(singleGame, {"A", "B"});
    singleGame.accessRuleConfig().setSinglesEnabled(false);
    setDice(singleGame, {{1, 1}, {5, 1}});
    const Checker singleChecker(singleGame);
    options = singleChecker.getAvailableOptions();
    expect(!hasOption(options, "Score single 1 (100)"), "disabled singles should suppress single ones");
    expect(!hasOption(options, "Score single 5 (50)"), "disabled singles should suppress single fives");
    expect(!singleChecker.isOptionAvailable(), "only singles should mean no options when singles are disabled");
}

void firstRollBustBonusIsOneTimeAndRestartsRoll()
{
    GameManager game;
    prepareTurn(game, {"A", "B"});
    setDice(game, {{2, 2}, {3, 1}, {4, 2}, {6, 1}});
    game.registerRoll();

    const Checker checker(game);
    expect(!checker.isOptionAvailable(), "test bust hand should have no scoring options");
    checker.handleBusts();

    Score& score = game.getCurrentPlayer()->getScore();
    expectEqual(score.getRoundScore(), static_cast<uint32_t>(50), "first-roll bust bonus should add 50 points");
    expectEqual(game.getCurrentPlayer()->getDice().getNumDiceInPlay(), static_cast<uint16_t>(FULL_SET_OF_DICE), "first-roll bust bonus should reset dice");
    expect(game.getBustBonusUsedThisTurn(), "first-roll bust bonus should be marked used");
    expect(!game.getHasRolledThisTurn(), "first-roll bust bonus should allow a fresh roll");
    expect(!game.getSelectedOptionStatus(), "first-roll bust bonus should not count as a score selection");
    expect(!game.getBustPending(), "first-roll bust bonus should not leave a pending bust");
    expect(game.getTurnContinuationStatus(), "first-roll bust bonus should continue the turn");

    setDice(game, {{2, 2}, {3, 1}, {4, 2}, {6, 1}});
    game.registerRoll();
    checker.handleBusts();
    expectEqual(score.getRoundScore(), static_cast<uint32_t>(0), "a second bust should clear the round score");
    expect(game.getBustPending(), "a second bust should require pass acknowledgment");
    expect(!game.getTurnContinuationStatus(), "a second bust should end the turn");
}

void disabledBustBonusBustsImmediately()
{
    GameManager game;
    prepareTurn(game, {"A", "B"});
    game.accessRuleConfig().setFirstRollBustBonusEnabled(false);
    game.getCurrentPlayer()->getScore().increaseRoundScore(300);
    setDice(game, {{2, 2}, {3, 1}, {4, 2}, {6, 1}});
    game.registerRoll();

    const Checker checker(game);
    checker.handleBusts();

    expectEqual(game.getCurrentPlayer()->getScore().getRoundScore(), static_cast<uint32_t>(0), "regular bust should clear accumulated round score");
    expect(game.getBustPending(), "regular bust should mark a pending bust");
    expect(!game.getTurnContinuationStatus(), "regular bust should stop the turn");
}

void nonBustHandsDoNotTriggerBustHandling()
{
    GameManager game;
    prepareTurn(game, {"A", "B"});
    setDice(game, {{1, 1}, {2, 1}, {3, 1}, {4, 1}, {6, 2}});
    game.registerRoll();

    const Checker checker(game);
    expect(checker.isOptionAvailable(), "test hand should have a scoring option");
    checker.handleBusts();

    expectEqual(game.getCurrentPlayer()->getScore().getRoundScore(), static_cast<uint32_t>(0), "non-bust hand should not receive a bust bonus");
    expect(!game.getBustBonusUsedThisTurn(), "non-bust hand should not consume the bust bonus");
    expect(!game.getBustPending(), "non-bust hand should not mark a bust");
    expect(game.getTurnContinuationStatus(), "non-bust hand should keep the turn active");
}

void emptyDiceAfterScoringResetsToHotDice()
{
    GameManager game;
    prepareTurn(game, {"A", "B"});
    Dice& dice = game.getCurrentPlayer()->getDice();
    dice.diceSetMap.clear();
    dice.setNumDiceInPlay(0);

    const Checker checker(game);
    checker.handleBusts();

    expectEqual(dice.getNumDiceInPlay(), static_cast<uint16_t>(FULL_SET_OF_DICE), "empty dice state should reset to hot dice");
    expect(game.getTurnContinuationStatus(), "empty dice reset should keep the turn alive");
}

void bankingRequiresRollSelectionScoreAndThreshold()
{
    GameManager game;
    prepareTurn(game, {"A", "B"});
    Score& score = game.getCurrentPlayer()->getScore();
    score.increaseRoundScore(1000);
    game.setSelectedOptionStatus(true);
    expect(!game.canBankCurrentScore(), "banking should require a roll");

    game.registerRoll();
    game.setSelectedOptionStatus(false);
    expect(!game.canBankCurrentScore(), "banking should require selecting a score option");

    game.setSelectedOptionStatus(true);
    game.accessRuleConfig().setBankThreshold(2000);
    expect(!game.canBankCurrentScore(), "banking should require meeting the threshold");

    score.increasePermanentScore(950);
    expect(!game.canBankCurrentScore(), "banking should still fail just below threshold");
    score.increasePermanentScore(50);
    expect(game.canBankCurrentScore(), "banking should pass at the threshold");

    game.setTurnContinuationStatus(false, true);
    expect(!game.canBankCurrentScore(), "banking should be blocked while a bust is pending");
}

void endTurnAndBankMovesRoundScoreOnlyWhenAllowed()
{
    GameManager game;
    prepareTurn(game, {"A", "B"});
    Score& score = game.getCurrentPlayer()->getScore();
    score.increasePermanentScore(900);
    score.increaseRoundScore(100);
    game.registerRoll();
    game.setSelectedOptionStatus(true);

    game.endTurnAndBank();
    expectEqual(score.getPermanentScore(), static_cast<uint32_t>(1000), "banking should add round score to permanent score");
    expectEqual(score.getRoundScore(), static_cast<uint32_t>(0), "banking should clear round score");
    expectEqual(game.getCurrentPlayer()->getDice().getNumDiceInPlay(), static_cast<uint16_t>(0), "banking should leave no active dice for this turn");
    expect(!game.getTurnContinuationStatus(), "banking should end the turn");
    expect(!game.getSelectionContinuationStatus(), "banking should end selection");

    GameManager blocked;
    prepareTurn(blocked, {"A", "B"});
    blocked.getCurrentPlayer()->getScore().increaseRoundScore(999);
    blocked.registerRoll();
    blocked.setSelectedOptionStatus(true);
    blocked.endTurnAndBank();
    expectEqual(blocked.getCurrentPlayer()->getScore().getPermanentScore(), static_cast<uint32_t>(0), "blocked bank should not change permanent score");
    expectEqual(blocked.getCurrentPlayer()->getScore().getRoundScore(), static_cast<uint32_t>(999), "blocked bank should not clear round score");
}

void finalRoundRequiresBeatingLeaderWhenTiesDisabled()
{
    GameManager game;
    game.setPlayers({"A", "B", "C"});
    game.setScoreLimit(5000);
    game.accessRuleConfig().setAllowTies(false);

    game.accessPlayers()[1].getScore().increasePermanentScore(5000);
    game.startTurn(1);
    game.startFinalRound();
    expect(game.getFinalRoundLeaderIndex() == 1, "final-round starter should start as leader");

    game.accessPlayers()[0].getScore().increasePermanentScore(5000);
    game.startTurn(0);
    game.updateFinalRoundLeaderForCurrentPlayer();
    expect(game.getFinalRoundLeaderIndex() == 1, "tying the leader should not replace them when ties are disabled");

    game.accessPlayers()[2].getScore().increasePermanentScore(5100);
    game.startTurn(2);
    game.updateFinalRoundLeaderForCurrentPlayer();
    expect(game.getFinalRoundLeaderIndex() == 2, "beating the leader should replace them");
}

void turnOrderFinalRoundAndResetStateWrapSafely()
{
    GameManager game;
    game.setPlayers({"A", "B", "C"});
    game.setScoreLimit(5000);

    game.startTurn(5);
    expectEqual(game.getCurrentIndex(), static_cast<std::size_t>(2), "startTurn should wrap out-of-range indexes");
    expectEqual(game.getCurrentPlayer()->getName(), std::string("C"), "wrapped turn should select the expected player");

    game.switchToNextPlayer();
    expectEqual(game.getCurrentIndex(), static_cast<std::size_t>(0), "switchToNextPlayer should wrap to the first player");

    game.startFinalRound();
    game.startTurn(1);
    expect(!game.willEndAfterCurrentTurn(game.getPlayers().size()), "middle player should not end the final round");
    game.startTurn(2);
    expect(game.willEndAfterCurrentTurn(game.getPlayers().size()), "last player before starter should end the final round");

    game.setPlayers({"Only"});
    expect(!game.isFinalRoundActive(), "resetting players should clear final-round state");
    expectEqual(game.getFinalRoundStarterIndex(), static_cast<std::size_t>(0), "resetting players should clear starter index");
    expectEqual(game.getFinalRoundLeaderIndex(), static_cast<std::size_t>(0), "resetting players should clear leader index");
}

void selectionAndSavedMultipleStateResetCorrectly()
{
    GameManager game;
    prepareTurn(game, {"A", "B"});

    game.setSelectionContinuationStatus(false);
    expect(game.getSelectionContinuationStatus(), "selection cannot end before any option is selected");

    game.setSelectedOptionStatus(true);
    game.setSelectionContinuationStatus(false);
    expect(!game.getSelectionContinuationStatus(), "selection can end after an option is selected");

    game.setValueOfChosenMultiple(5);
    game.getCurrentPlayer()->getScore().setScoreFromMultiples(400);
    game.resetSavedMultiple();
    expectEqual(game.getValueOfChosenMultiple(), static_cast<uint16_t>(0), "resetSavedMultiple should clear the saved die value");
    expectEqual(game.getCurrentPlayer()->getScore().getScoreFromMultiples(), static_cast<uint16_t>(0), "resetSavedMultiple should clear saved multiple score");
}

void ruleConfigTogglesAndThresholdsAreStable()
{
    RuleConfig config;
    config.toggleStraits();
    config.toggleSets();
    config.toggleMultiples();
    config.toggleSingles();
    config.toggleAnimations();
    config.toggleFirstRollBustBonus();
    config.toggleFinalChase();
    config.toggleAllowTies();
    expect(!config.straitsEnabled(), "straits toggle should disable straights");
    expect(!config.setsEnabled(), "sets toggle should disable sets");
    expect(!config.multiplesEnabled(), "multiples toggle should disable multiples");
    expect(!config.singlesEnabled(), "singles toggle should disable singles");
    expect(!config.animationsEnabled(), "animations toggle should disable animations");
    expect(!config.firstRollBustBonusEnabled(), "first-roll bust toggle should disable the bonus");
    expect(!config.finalChaseEnabled(), "final chase toggle should disable final chase");
    expect(config.tiesAllowed(), "allow ties toggle should enable ties");

    config.setStraitsEnabled(true);
    config.setSetsEnabled(true);
    config.setMultiplesEnabled(true);
    config.setSinglesEnabled(true);
    config.setFirstRollBustBonusEnabled(true);
    config.setFinalChaseEnabled(true);
    config.setAllowTies(false);
    expect(config.straitsEnabled(), "straits setter should enable straights");
    expect(config.setsEnabled(), "sets setter should enable sets");
    expect(config.multiplesEnabled(), "multiples setter should enable multiples");
    expect(config.singlesEnabled(), "singles setter should enable singles");
    expect(config.firstRollBustBonusEnabled(), "first-roll bust setter should enable the bonus");
    expect(config.finalChaseEnabled(), "final chase setter should enable final chase");
    expect(!config.tiesAllowed(), "allow ties setter should disable ties");

    config.adjustBankThreshold(-2000);
    expectEqual(config.getBankThreshold(), static_cast<uint32_t>(0), "bank threshold should clamp at zero");
    config.adjustBankThreshold(250);
    expectEqual(config.getBankThreshold(), static_cast<uint32_t>(250), "bank threshold should increase from zero");
}

void winnerLookupHandlesEmptyAndHighestScore()
{
    GameManager game;
    expect(game.getWinner() == nullptr, "empty game should have no winner");
    expect(game.findHighestScoringPlayer() == nullptr, "empty game should have no highest-scoring player");

    game.setPlayers({"A", "B", "C"});
    game.accessPlayers()[0].getScore().increasePermanentScore(700);
    game.accessPlayers()[1].getScore().increasePermanentScore(1200);
    game.accessPlayers()[2].getScore().increasePermanentScore(900);

    Player* highest = game.findHighestScoringPlayer();
    const Player* winner = game.getWinner();
    expect(highest != nullptr, "highest-scoring player should exist after players are configured");
    expect(winner != nullptr, "winner should exist after players are configured");
    expectEqual(highest->getName(), std::string("B"), "findHighestScoringPlayer should return the highest permanent score");
    expectEqual(winner->getName(), std::string("B"), "getWinner should return the highest permanent score");
}

void resourcePathLeavesAbsoluteAndMissingRelativePathsStable()
{
    const std::filesystem::path cwd = std::filesystem::current_path();
    expectEqual(resolveResourcePath(cwd), cwd.string(), "absolute resource paths should be returned unchanged");

    const std::filesystem::path missing = "definitely-not-a-zilch-resource.file";
    expectEqual(resolveResourcePath(missing), missing.string(), "missing relative resource paths should remain relative");
}

void emptyPlayerResetIsSafe()
{
    GameManager game;
    prepareTurn(game, {"A", "B"});

    game.setPlayers({});
    expect(game.getPlayers().empty(), "empty player reset should clear players");
    expect(game.getCurrentPlayer() == nullptr, "empty player reset should clear current player");

    game.manageDiceCount(3);
    expect(game.getCurrentPlayer() == nullptr, "managing dice without players should remain safe");
}
}

int main()
{
    runTest("diceRollMaintainsConsistentState", diceRollMaintainsConsistentState);
    runTest("scoreAndPlayerDefaultsAreStable", scoreAndPlayerDefaultsAreStable);
    runTest("gameStateMachineDescribesEveryState", gameStateMachineDescribesEveryState);
    runTest("singlesDoNotMasqueradeAsMultiples", singlesDoNotMasqueradeAsMultiples);
    runTest("singlesScoreOnlyOneDie", singlesScoreOnlyOneDie);
    runTest("straightsAndSetsScoreHotDice", straightsAndSetsScoreHotDice);
    runTest("multiplesScoreScaleAndCanBeExtended", multiplesScoreScaleAndCanBeExtended);
    runTest("fourOnesDoubleBaseMultipleAndHotDice", fourOnesDoubleBaseMultipleAndHotDice);
    runTest("ruleManagerExposesStructuredGameOptions", ruleManagerExposesStructuredGameOptions);
    runTest("ruleTogglesSuppressOnlyTheirConfiguredOptions", ruleTogglesSuppressOnlyTheirConfiguredOptions);
    runTest("firstRollBustBonusIsOneTimeAndRestartsRoll", firstRollBustBonusIsOneTimeAndRestartsRoll);
    runTest("disabledBustBonusBustsImmediately", disabledBustBonusBustsImmediately);
    runTest("nonBustHandsDoNotTriggerBustHandling", nonBustHandsDoNotTriggerBustHandling);
    runTest("emptyDiceAfterScoringResetsToHotDice", emptyDiceAfterScoringResetsToHotDice);
    runTest("bankingRequiresRollSelectionScoreAndThreshold", bankingRequiresRollSelectionScoreAndThreshold);
    runTest("endTurnAndBankMovesRoundScoreOnlyWhenAllowed", endTurnAndBankMovesRoundScoreOnlyWhenAllowed);
    runTest("finalRoundRequiresBeatingLeaderWhenTiesDisabled", finalRoundRequiresBeatingLeaderWhenTiesDisabled);
    runTest("turnOrderFinalRoundAndResetStateWrapSafely", turnOrderFinalRoundAndResetStateWrapSafely);
    runTest("selectionAndSavedMultipleStateResetCorrectly", selectionAndSavedMultipleStateResetCorrectly);
    runTest("ruleConfigTogglesAndThresholdsAreStable", ruleConfigTogglesAndThresholdsAreStable);
    runTest("winnerLookupHandlesEmptyAndHighestScore", winnerLookupHandlesEmptyAndHighestScore);
    runTest("resourcePathLeavesAbsoluteAndMissingRelativePathsStable", resourcePathLeavesAbsoluteAndMissingRelativePathsStable);
    runTest("emptyPlayerResetIsSafe", emptyPlayerResetIsSafe);

    std::cout << "All rule tests passed.\n";
    return EXIT_SUCCESS;
}
