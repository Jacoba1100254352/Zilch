#include "Checker.h"
#include "ComputerStrategy.h"
#include "Dice.h"
#include "GameManager.h"
#include "GameStateMachine.h"
#include "Player.h"
#include "ResourcePath.h"
#include "Score.h"
#include "rules/RuleManager.h"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <iostream>
#include <stdexcept>
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

void prepareComputerTurn(GameManager& game, const ComputerDifficulty difficulty)
{
    game.setPlayerSetups({
        {"Computer", PlayerKind::Computer, difficulty},
        {"Opponent", PlayerKind::Human, ComputerDifficulty::Medium}
    });
    game.setScoreLimit(5000);
    game.startTurn(0);
}

void prepareStealDecision(
    GameManager& game,
    const std::uint32_t inheritedScore,
    const std::uint16_t diceCount,
    const ComputerDifficulty difficulty)
{
    game.setPlayerSetups({
        {"Offerer", PlayerKind::Human, ComputerDifficulty::Medium},
        {"Computer", PlayerKind::Computer, difficulty}
    });
    game.setScoreLimit(5000);
    game.accessRuleConfig().setStealingEnabled(true);
    game.startTurn(0);
    game.accessPlayers()[0].getScore().increasePermanentScore(1000);
    game.accessPlayers()[1].getScore().increasePermanentScore(1000);

    game.getCurrentPlayer()->getScore().increaseRoundScore(inheritedScore);
    game.getCurrentPlayer()->getDice().setNumDiceInPlay(diceCount);
    game.registerRoll();
    game.setSelectedOptionStatus(true);
    game.endTurnAndBank();
    game.switchToNextPlayer();
    game.startTurn(game.getCurrentIndex());
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
    expectEqual(score.getScoreLimit(), static_cast<uint32_t>(5000), "score should default to the shared winning score");
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

void mixedHumanAndComputerPlayerSetupIsPreserved()
{
    GameManager game;
    game.setPlayerSetups({
        {"Alice", PlayerKind::Human, ComputerDifficulty::Medium},
        {"Easy CPU", PlayerKind::Computer, ComputerDifficulty::Easy},
        {"Hard CPU", PlayerKind::Computer, ComputerDifficulty::Hard}
    });

    expectEqual(game.getPlayers().size(), static_cast<std::size_t>(3), "mixed setup should preserve every player slot");
    expect(!game.getPlayers()[0].isComputer(), "human setup should remain human");
    expect(game.getPlayers()[1].isComputer(), "computer setup should remain a computer");
    expect(game.getPlayers()[1].getComputerDifficulty() == ComputerDifficulty::Easy, "easy level should be retained");
    expect(game.getPlayers()[2].getComputerDifficulty() == ComputerDifficulty::Hard, "hard level should be retained");

    game.setPlayers({"One", "Two", "Three", "Four", "Five", "Six"});
    expectEqual(game.getPlayers().size(), static_cast<std::size_t>(6), "legacy setup should still support six players");
    for (const Player& player : game.getPlayers())
        expect(!player.isComputer(), "legacy name-only setup should continue creating human players");
}

void computerPoliciesMatchTheTestedProfiles()
{
    using zilch::computer::policyForDifficulty;

    const auto& easy = policyForDifficulty(ComputerDifficulty::Easy, false);
    expect(easy.bankThresholdByDice == std::array<int, 7>{0, 600, 600, 600, 600, 600, 600},
           "easy should use a fixed 600-point banking target");

    const auto& hard = policyForDifficulty(ComputerDifficulty::Hard, false);
    expect(hard.bankThresholdByDice == std::array<int, 7>{0, 200, 1021, 1128, 1506, 2130, 5000},
           "standard hard thresholds should match the simulator champion");
    expectEqual(hard.scoreWeight, 1.0045, "standard hard score weight should match the trained policy");
    expectEqual(hard.remainingDiceWeight, 36.0805, "standard hard dice weight should match the trained policy");
    expectEqual(hard.hotDiceWeight, 354.561, "standard hard hot-dice weight should match the trained policy");
    expectEqual(hard.multipleWeight, 91.9329, "standard hard multiple weight should match the trained policy");
    expectEqual(hard.leadFactor, 0.0, "standard hard lead factor should match the trained policy");
    expectEqual(hard.trailFactor, 0.293194, "standard hard trail factor should match the trained policy");
    expectEqual(hard.closingFactor, 0.193316, "standard hard closing factor should match the trained policy");
    expectEqual(hard.rollBias, 136.066, "standard hard roll bias should match the trained policy");

    const auto& stealing = policyForDifficulty(ComputerDifficulty::Hard, true);
    expect(stealing.bankThresholdByDice == std::array<int, 7>{0, 313, 313, 1106, 1360, 1360, 1376},
           "Stealing hard thresholds should use the separately trained policy");
    expectEqual(stealing.scoreWeight, 0.88553, "Stealing hard score weight should match the trained policy");
    expectEqual(stealing.remainingDiceWeight, 91.2663, "Stealing hard dice weight should match the trained policy");
    expectEqual(stealing.hotDiceWeight, 229.628, "Stealing hard hot-dice weight should match the trained policy");
    expectEqual(stealing.multipleWeight, 94.2546, "Stealing hard multiple weight should match the trained policy");
    expectEqual(stealing.leadFactor, 0.0, "Stealing hard lead factor should match the trained policy");
    expectEqual(stealing.trailFactor, 0.187935, "Stealing hard trail factor should match the trained policy");
    expectEqual(stealing.closingFactor, 0.20764, "Stealing hard closing factor should match the trained policy");
    expectEqual(stealing.rollBias, -26.2974, "Stealing hard roll bias should match the trained policy");
}

void easyScoresEveryAvailableDieBeforeRolling()
{
    GameManager game;
    prepareComputerTurn(game, ComputerDifficulty::Easy);
    game.registerRoll();
    setDice(game, {{1, 1}, {2, 1}, {5, 2}});
    zilch::rules::RuleManager rules(game.getRuleConfig());

    std::size_t selected = 0;
    while (true) {
        const auto options = rules.evaluate(game);
        if (options.empty())
            break;
        const std::size_t index = zilch::computer::chooseOption(game, options);
        rules.apply(game, options[index]);
        ++selected;

        const auto remaining = rules.evaluate(game);
        if (remaining.empty())
            break;
        expect(zilch::computer::decideAfterSelection(game, remaining) ==
               zilch::computer::PostSelectionDecision::SelectAgain,
               "easy should keep selecting while any scoring die remains");
    }

    expectEqual(selected, static_cast<std::size_t>(3), "easy should select the one and both fives");
    expectEqual(game.getCurrentPlayer()->getScore().getRoundScore(), static_cast<uint32_t>(200),
                "easy should collect every available single score");
    expectEqual(game.getCurrentPlayer()->getDice().getNumDiceInPlay(), static_cast<uint16_t>(1),
                "only the non-scoring die should remain after easy auto-selection");
}

void easyBanksAtItsFixedThreshold()
{
    GameManager game;
    prepareComputerTurn(game, ComputerDifficulty::Easy);
    game.getCurrentPlayer()->getScore().increasePermanentScore(1000);
    game.getCurrentPlayer()->getScore().increaseRoundScore(550);
    game.getCurrentPlayer()->getDice().setNumDiceInPlay(2);
    game.registerRoll();
    game.setSelectedOptionStatus(true);

    expect(!zilch::computer::shouldBank(game), "easy should keep rolling below 600 round points");
    game.getCurrentPlayer()->getScore().increaseRoundScore(50);
    expect(zilch::computer::shouldBank(game), "easy should bank once its round reaches 600 points");
}

void hardCanPreserveAWeakScoringDieForTheReroll()
{
    GameManager game;
    prepareComputerTurn(game, ComputerDifficulty::Hard);
    game.accessRuleConfig().setStraightEnabled(false);
    game.registerRoll();
    setDice(game, {{1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}, {6, 1}});
    zilch::rules::RuleManager rules(game.getRuleConfig());

    auto options = rules.evaluate(game);
    const std::size_t firstChoice = zilch::computer::chooseOption(game, options);
    expectEqual(options[firstChoice].selectedValue, static_cast<uint16_t>(1),
                "hard should prefer the stronger single one first");
    rules.apply(game, options[firstChoice]);

    options = rules.evaluate(game);
    expectEqual(options.size(), static_cast<std::size_t>(1), "only the weak single five should remain");
    expect(zilch::computer::decideAfterSelection(game, options) == zilch::computer::PostSelectionDecision::Roll,
           "hard should reroll five dice instead of consuming a weak final scoring option");
}

void hardSwitchesTheWholePolicyForStealing()
{
    GameManager scoringGame;
    prepareComputerTurn(scoringGame, ComputerDifficulty::Hard);
    scoringGame.registerRoll();
    scoringGame.setSelectedOptionStatus(true);
    setDice(scoringGame, {{1, 1}, {2, 1}, {3, 1}, {4, 1}, {6, 1}});
    zilch::rules::RuleManager standardRules(scoringGame.getRuleConfig());
    const auto remainingOptions = standardRules.evaluate(scoringGame);
    expect(zilch::computer::decideAfterSelection(scoringGame, remainingOptions) ==
           zilch::computer::PostSelectionDecision::Roll,
           "standard hard should reroll rather than consume the remaining one");

    scoringGame.accessRuleConfig().setStealingEnabled(true);
    expect(zilch::computer::decideAfterSelection(scoringGame, remainingOptions) ==
           zilch::computer::PostSelectionDecision::SelectAgain,
           "Stealing hard should use its different utility weights for scoring choices");

    GameManager standardBankGame;
    prepareComputerTurn(standardBankGame, ComputerDifficulty::Hard);
    standardBankGame.getCurrentPlayer()->getScore().increasePermanentScore(1000);
    standardBankGame.accessPlayers()[1].getScore().increasePermanentScore(1000);
    standardBankGame.getCurrentPlayer()->getScore().increaseRoundScore(1400);
    standardBankGame.getCurrentPlayer()->getDice().setNumDiceInPlay(4);
    standardBankGame.registerRoll();
    standardBankGame.setSelectedOptionStatus(true);
    expect(!zilch::computer::shouldBank(standardBankGame),
           "standard hard should keep rolling below its four-dice threshold");

    GameManager stealingBankGame;
    prepareComputerTurn(stealingBankGame, ComputerDifficulty::Hard);
    stealingBankGame.accessRuleConfig().setStealingEnabled(true);
    stealingBankGame.getCurrentPlayer()->getScore().increasePermanentScore(1000);
    stealingBankGame.accessPlayers()[1].getScore().increasePermanentScore(1000);
    stealingBankGame.getCurrentPlayer()->getScore().increaseRoundScore(1400);
    stealingBankGame.getCurrentPlayer()->getDice().setNumDiceInPlay(4);
    stealingBankGame.registerRoll();
    stealingBankGame.setSelectedOptionStatus(true);
    expect(zilch::computer::shouldBank(stealingBankGame),
           "Stealing hard should bank at its lower four-dice threshold");
}

void hardBankThresholdKeepsFractionalPrecision()
{
    GameManager game;
    prepareComputerTurn(game, ComputerDifficulty::Hard);
    game.accessPlayers()[1].getScore().increasePermanentScore(1100);
    game.getCurrentPlayer()->getScore().increaseRoundScore(1450);
    game.getCurrentPlayer()->getDice().setNumDiceInPlay(3);
    game.registerRoll();
    game.setSelectedOptionStatus(true);

    expect(!zilch::computer::shouldBank(game),
           "1450 points must stay below the full-precision 1450.5134 threshold");
    game.getCurrentPlayer()->getScore().increaseRoundScore(50);
    expect(zilch::computer::shouldBank(game),
           "hard should bank at the next reachable 50-point score above its threshold");
}

void hardRollsTheReported2800HotDiceState()
{
    GameManager game;
    prepareComputerTurn(game, ComputerDifficulty::Hard);
    game.registerRoll();
    game.registerRoll();
    game.setSelectedOptionStatus(true);
    game.getCurrentPlayer()->getScore().setRoundScore(2800);
    game.getCurrentPlayer()->getDice().setNumDiceInPlay(FULL_SET_OF_DICE);

    expect(!game.isFinalRoundActive(), "the reported fresh game must not be in Final Chase");
    expect(!zilch::computer::shouldBank(game), "Hard should roll six hot dice at 2800 with both totals zero");
    expect(zilch::computer::chooseTurnAction(game, true, false, {}) == zilch::computer::TurnAction::Roll,
           "the actual visual action planner should roll the reported hot-dice state");

    game.accessRuleConfig().setStealingEnabled(true);
    expect(zilch::computer::shouldBank(game), "the existing Stealing six-dice decision must still bank");
}

void hardCollectsGuaranteedPointsBeforeBanking()
{
    using zilch::computer::TurnAction;
    GameManager game;
    prepareComputerTurn(game, ComputerDifficulty::Hard);
    game.registerRoll();
    game.getCurrentPlayer()->getScore().setRoundScore(2700);
    setDice(game, {{1, 2}, {5, 1}, {2, 1}, {3, 1}, {4, 1}});
    zilch::rules::RuleManager rules(game.getRuleConfig());
    auto options = rules.evaluate(game);
    rules.apply(game, options[zilch::computer::chooseOption(game, options)]);
    expectEqual(game.getCurrentPlayer()->getScore().getRoundScore(), 2800U,
                "the original rolling plan should first select one scoring one");

    options = rules.evaluate(game);
    expect(zilch::computer::chooseTurnAction(game, false, true, options) == TurnAction::SelectOption,
           "a banking decision should collect the unclaimed guaranteed points first");
    expect(game.getComputerBankCommitted(), "collection must explicitly commit to banking");
    for (int action = 0; action < 2; ++action) {
        options = rules.evaluate(game);
        rules.apply(game, options[zilch::computer::chooseOption(game, options)]);
        options = rules.evaluate(game);
        expect(zilch::computer::chooseTurnAction(game, false, true, options) ==
                   (options.empty() ? TurnAction::Bank : TurnAction::SelectOption),
               "the collector should finish the scoring options then bank without another roll");
    }
    expectEqual(game.getCurrentPlayer()->getScore().getRoundScore(), 2950U,
                "Hard must bank 2950 rather than leave 150 guaranteed points behind");
    game.endTurnAndBank();
    expectEqual(game.getCurrentPlayer()->getScore().getPermanentScore(), 2950U,
                "the complete collected score should reach the scoreboard");
    expect(!game.getComputerBankCommitted(), "banking should clear the per-turn commitment");
}

void hardBankCommitmentSurvivesHotDice()
{
    using zilch::computer::PostSelectionDecision;
    GameManager game;
    prepareComputerTurn(game, ComputerDifficulty::Hard);
    game.registerRoll();
    game.setSelectedOptionStatus(true);
    game.getCurrentPlayer()->getScore().setRoundScore(2800);
    setDice(game, {{1, 1}, {5, 1}});
    zilch::rules::RuleManager rules(game.getRuleConfig());
    auto options = rules.evaluate(game);
    expect(zilch::computer::decideAfterSelection(game, options) == PostSelectionDecision::SelectAgain,
           "the two-dice bank plan should begin collecting both guaranteed singles");
    expect(game.getComputerBankCommitted(), "the original two-dice decision must be remembered");
    expectEqual(options[zilch::computer::chooseOption(game, options)].pointsAwarded, 100U,
                "bank collection should take the highest-scoring remaining option");
    for (int action = 0; action < 2; ++action) {
        options = rules.evaluate(game);
        rules.apply(game, options[zilch::computer::chooseOption(game, options)]);
    }
    expectEqual(game.getCurrentPlayer()->getDice().getNumDiceInPlay(), FULL_SET_OF_DICE,
                "collecting every scoring die should reset to six hot dice");
    expect(game.getComputerBankCommitted(), "hot dice must not clear a bank commitment");
    expectEqual(game.getCurrentPlayer()->getScore().getRoundScore(), 2950U,
                "the collector should include both singles");
    expect(zilch::computer::chooseTurnAction(game, true, false, {}) == zilch::computer::TurnAction::Bank,
           "the UI's hot-dice waiting state must bank rather than apply the new six-dice cutoff");
}

void hardCollectsBeforeImmediateWinAndFinalChaseBank()
{
    for (const bool finalChase : {false, true}) {
        GameManager game;
        prepareComputerTurn(game, ComputerDifficulty::Hard);
        if (finalChase) {
            game.getCurrentPlayer()->getScore().increasePermanentScore(4500);
            game.accessPlayers()[1].getScore().increasePermanentScore(5200);
            game.startFinalRound();
        } else {
            game.accessRuleConfig().setFinalChaseEnabled(false);
        }
        game.registerRoll();
        game.getCurrentPlayer()->getScore().setRoundScore(finalChase ? 600 : 4900);
        setDice(game, {{1, 2}, {5, 1}, {2, 1}, {3, 1}, {4, 1}});
        zilch::rules::RuleManager rules(game.getRuleConfig());
        auto options = rules.evaluate(game);
        rules.apply(game, options[zilch::computer::chooseOption(game, options)]);
        for (int action = 0; action < 2; ++action) {
            options = rules.evaluate(game);
            expect(zilch::computer::decideAfterSelection(game, options) ==
                       zilch::computer::PostSelectionDecision::SelectAgain,
                   "a winning bank should collect free points without rolling again");
            expect(game.getComputerBankCommitted(), "winning bank intent must be committed");
            rules.apply(game, options[zilch::computer::chooseOption(game, options)]);
        }
        expect(zilch::computer::decideAfterSelection(game, rules.evaluate(game)) ==
                   zilch::computer::PostSelectionDecision::Bank,
               "the final action must remain a bank after collecting winning points");
        game.endTurnAndBank();
        expectEqual(game.getCurrentPlayer()->getScore().getPermanentScore(), finalChase ? 5350U : 5150U,
                    "a winning bank should include every guaranteed point");
    }
}

void computerBankCommitmentResetsAtTurnBoundaries()
{
    GameManager game;
    prepareComputerTurn(game, ComputerDifficulty::Hard);
    game.setComputerBankCommitted(true);
    game.registerRoll();
    expect(!game.getComputerBankCommitted(), "an actual new roll must clear stale bank intent");
    game.setComputerBankCommitted(true);
    game.setTurnContinuationStatus(false, true);
    expect(!game.getComputerBankCommitted(), "a bust must clear bank intent");
    game.setComputerBankCommitted(true);
    game.startTurn(0);
    expect(!game.getComputerBankCommitted(), "starting a turn must clear bank intent");
    game.setComputerBankCommitted(true);
    game.setTurnContinuationStatus(false);
    expect(!game.getComputerBankCommitted(), "a canceled turn must clear bank intent");
    game.setComputerBankCommitted(true);
    game.switchToNextPlayer();
    expect(!game.getComputerBankCommitted(), "switching players must clear bank intent");
    game.setComputerBankCommitted(true);
    game.setPlayers({"New player"});
    expect(!game.getComputerBankCommitted(), "creating a new game must clear bank intent");

    GameManager otherGame;
    prepareComputerTurn(otherGame, ComputerDifficulty::Hard);
    game.setComputerBankCommitted(true);
    expect(!otherGame.getComputerBankCommitted(), "separate games must never share computer intent");
}

void stealingHardDoesNotUseBankCollector()
{
    GameManager game;
    prepareComputerTurn(game, ComputerDifficulty::Hard);
    game.accessRuleConfig().setStealingEnabled(true);
    game.registerRoll();
    game.setSelectedOptionStatus(true);
    game.getCurrentPlayer()->getScore().setRoundScore(2800);
    setDice(game, {{5, 1}, {2, 1}, {3, 1}, {4, 1}, {6, 1}});
    zilch::rules::RuleManager rules(game.getRuleConfig());
    const auto options = rules.evaluate(game);
    expect(zilch::computer::decideAfterSelection(game, options) ==
               zilch::computer::PostSelectionDecision::Bank,
           "Stealing Hard must retain its bank with an unclaimed weak scoring die");
    expect(!game.getComputerBankCommitted(), "Stealing must not enable the new collector");
}

void mediumUsesOpponentAwareFinishDecisions()
{
    GameManager bufferGame;
    prepareComputerTurn(bufferGame, ComputerDifficulty::Medium);
    bufferGame.getCurrentPlayer()->getScore().increasePermanentScore(4900);
    bufferGame.accessPlayers()[1].getScore().increasePermanentScore(4700);
    bufferGame.getCurrentPlayer()->getScore().increaseRoundScore(100);
    bufferGame.getCurrentPlayer()->getDice().setNumDiceInPlay(4);
    bufferGame.registerRoll();
    bufferGame.setSelectedOptionStatus(true);
    expect(!zilch::computer::shouldBank(bufferGame),
           "medium should press for a buffer when a close opponent would receive a final chase");

    GameManager saferDiceGame;
    prepareComputerTurn(saferDiceGame, ComputerDifficulty::Medium);
    saferDiceGame.getCurrentPlayer()->getScore().increasePermanentScore(4900);
    saferDiceGame.accessPlayers()[1].getScore().increasePermanentScore(4700);
    saferDiceGame.getCurrentPlayer()->getScore().increaseRoundScore(100);
    saferDiceGame.getCurrentPlayer()->getDice().setNumDiceInPlay(3);
    saferDiceGame.registerRoll();
    saferDiceGame.setSelectedOptionStatus(true);
    expect(zilch::computer::shouldBank(saferDiceGame),
           "medium should take the win rather than chase a buffer with only three dice");

    GameManager stopShortGame;
    prepareComputerTurn(stopShortGame, ComputerDifficulty::Medium);
    stopShortGame.getCurrentPlayer()->getScore().increasePermanentScore(4500);
    stopShortGame.accessPlayers()[1].getScore().increasePermanentScore(4400);
    stopShortGame.getCurrentPlayer()->getScore().increaseRoundScore(450);
    stopShortGame.getCurrentPlayer()->getDice().setNumDiceInPlay(4);
    stopShortGame.registerRoll();
    stopShortGame.setSelectedOptionStatus(true);
    expect(zilch::computer::shouldBank(stopShortGame),
           "medium may stop just short when it already has a substantial lead");

    GameManager closeOpponentGame;
    prepareComputerTurn(closeOpponentGame, ComputerDifficulty::Medium);
    closeOpponentGame.getCurrentPlayer()->getScore().increasePermanentScore(4500);
    closeOpponentGame.accessPlayers()[1].getScore().increasePermanentScore(4700);
    closeOpponentGame.getCurrentPlayer()->getScore().increaseRoundScore(450);
    closeOpponentGame.getCurrentPlayer()->getDice().setNumDiceInPlay(4);
    closeOpponentGame.registerRoll();
    closeOpponentGame.setSelectedOptionStatus(true);
    expect(!zilch::computer::shouldBank(closeOpponentGame),
           "medium should not stop short when a close opponent makes that tactic unsound");
}

void computersTakeAReachableFinalChaseWin()
{
    GameManager winningGame;
    prepareComputerTurn(winningGame, ComputerDifficulty::Easy);
    winningGame.getCurrentPlayer()->getScore().increasePermanentScore(4500);
    winningGame.accessPlayers()[1].getScore().increasePermanentScore(5200);
    winningGame.getCurrentPlayer()->getScore().increaseRoundScore(800);
    winningGame.getCurrentPlayer()->getDice().setNumDiceInPlay(5);
    winningGame.registerRoll();
    winningGame.setSelectedOptionStatus(true);
    winningGame.startFinalRound();
    expect(zilch::computer::shouldBank(winningGame),
           "a computer should bank immediately after overtaking the final-chase leader");

    GameManager losingGame;
    prepareComputerTurn(losingGame, ComputerDifficulty::Easy);
    losingGame.getCurrentPlayer()->getScore().increasePermanentScore(4500);
    losingGame.accessPlayers()[1].getScore().increasePermanentScore(5200);
    losingGame.getCurrentPlayer()->getScore().increaseRoundScore(600);
    losingGame.getCurrentPlayer()->getDice().setNumDiceInPlay(5);
    losingGame.registerRoll();
    losingGame.setSelectedOptionStatus(true);
    losingGame.startFinalRound();
    expect(!zilch::computer::shouldBank(losingGame),
           "a computer should keep rolling while still behind in its last chance");
}

void hardUsesTheStealingSpecificAcceptanceCurve()
{
    constexpr std::array<std::uint32_t, 6> acceptanceMinimum{0, 550, 450, 350, 250, 150};
    for (std::uint16_t dice = 1; dice <= 5; ++dice) {
        const std::uint32_t minimum = acceptanceMinimum[dice];

        GameManager tooLow;
        prepareStealDecision(tooLow, minimum - 50, dice, ComputerDifficulty::Hard);
        expect(!zilch::computer::shouldAcceptSteal(tooLow),
               "hard should decline the score step below each Stealing-trained cutoff");

        GameManager enough;
        prepareStealDecision(enough, minimum, dice, ComputerDifficulty::Hard);
        expect(zilch::computer::shouldAcceptSteal(enough),
               "hard should accept at each Stealing-trained cutoff");
    }
}

void computerActionPlannerCoversTurnBoundaries()
{
    using zilch::computer::TurnAction;

    GameManager freshTurn;
    prepareComputerTurn(freshTurn, ComputerDifficulty::Easy);
    expect(zilch::computer::chooseTurnAction(freshTurn, true, false, {}) == TurnAction::Roll,
           "a fresh computer turn should start with a roll");

    freshTurn.setTurnContinuationStatus(false, true);
    expect(zilch::computer::chooseTurnAction(freshTurn, false, false, {}) == TurnAction::AcknowledgeBust,
           "a computer should acknowledge a visible bust before advancing");

    GameManager lowSteal;
    prepareStealDecision(lowSteal, 500, 1, ComputerDifficulty::Hard);
    expect(zilch::computer::chooseTurnAction(lowSteal, true, false, {}) == TurnAction::DeclineStealAndRoll,
           "hard should decline a continuation below its one-die cutoff and roll fresh");

    GameManager strongSteal;
    prepareStealDecision(strongSteal, 550, 1, ComputerDifficulty::Hard);
    expect(zilch::computer::chooseTurnAction(strongSteal, true, false, {}) == TurnAction::AcceptStealAndRoll,
           "hard should accept a continuation at its one-die cutoff before rolling");

    GameManager selection;
    prepareComputerTurn(selection, ComputerDifficulty::Easy);
    selection.accessRuleConfig().setOpeningScoreLimit(0);
    selection.registerRoll();
    setDice(selection, {{1, 1}, {2, 1}});
    zilch::rules::RuleManager ruleManager(selection.getRuleConfig());
    const auto options = ruleManager.evaluate(selection);
    expect(zilch::computer::chooseTurnAction(selection, false, true, options) == TurnAction::SelectOption,
           "a computer must score at least one available option after a roll");

    ruleManager.apply(selection, options.front());
    selection.getCurrentPlayer()->getScore().setRoundScore(600);
    expect(zilch::computer::chooseTurnAction(selection, false, true, {}) == TurnAction::Bank,
           "the planner should hand a bankable easy turn to the normal bank transition");

    selection.getCurrentPlayer()->getScore().setRoundScore(550);
    expect(zilch::computer::chooseTurnAction(selection, false, true, {}) == TurnAction::Roll,
           "the planner should keep an easy turn moving below its bank target");
}

void sharedDefaultsAndOpeningScoreAliasesAreStable()
{
    RuleConfig config;
    expect(config.straightEnabled() && config.straitsEnabled(), "Straight aliases should be enabled by default");
    expect(config.threePairsEnabled() && config.setsEnabled(), "Three Pairs aliases should be enabled by default");
    expect(config.multiplesEnabled(), "Multiples should be enabled by default");
    expect(config.singlesEnabled(), "Singles should be enabled by default");
    expect(config.firstRollBustBonusEnabled(), "First-Roll Bust should be enabled by default");
    expect(config.finalChaseEnabled(), "Final Chase should be enabled by default");
    expect(config.tiesAllowed(), "Allow Ties should be enabled by default");
    expect(!config.stealingEnabled(), "Stealing should be disabled by default");
    expect(config.hasScoringRuleEnabled(), "the shared defaults should include scoring rules");
    expectEqual(config.getOpeningScoreLimit(), static_cast<uint32_t>(1000), "Opening Score should default to 1000");
    expectEqual(config.getOpeningScore(), config.getOpeningScoreLimit(), "short getter should alias Opening Score Limit");
    expectEqual(config.getBankThreshold(), config.getOpeningScoreLimit(), "legacy getter should alias Opening Score Limit");

    config.setStraightEnabled(false);
    expect(!config.straitsEnabled(), "legacy Straight getter should reflect the canonical setter");
    config.toggleStraits();
    expect(config.straightEnabled(), "legacy Straight toggle should update the canonical state");
    config.setThreePairsEnabled(false);
    expect(!config.setsEnabled(), "legacy Three Pairs getter should reflect the canonical setter");
    config.toggleSets();
    expect(config.threePairsEnabled(), "legacy Three Pairs toggle should update the canonical state");

    config.setOpeningScoreLimit(750);
    expectEqual(config.getOpeningScore(), static_cast<uint32_t>(750), "short getter should see canonical setter changes");
    expectEqual(config.getBankThreshold(), static_cast<uint32_t>(750), "legacy getter should see canonical setter changes");
    config.adjustBankThreshold(250);
    expectEqual(config.getOpeningScoreLimit(), static_cast<uint32_t>(1000), "legacy adjuster should update Opening Score Limit");
    config.setBankThreshold(600);
    config.adjustOpeningScore(-100);
    expectEqual(config.getOpeningScoreLimit(), static_cast<uint32_t>(500), "short adjuster should update the canonical value");
    config.setOpeningScore(400);
    config.adjustOpeningScoreLimit(100);
    expectEqual(config.getBankThreshold(), static_cast<uint32_t>(500), "canonical adjuster should update compatibility getters");
}

void allDisabledScoringRulesAreDetected()
{
    RuleConfig config;
    config.setStraitsEnabled(false);
    config.setSetsEnabled(false);
    config.setMultiplesEnabled(false);
    config.setSinglesEnabled(false);
    expect(!config.hasScoringRuleEnabled(), "an all-disabled scoring configuration should be rejected by callers");

    config.setSinglesEnabled(true);
    expect(config.hasScoringRuleEnabled(), "enabling any scoring rule should make the configuration playable");
}

void openingScoreClampsToWinningScore()
{
    RuleConfig config;
    config.setOpeningScoreLimit(6000);
    config.clampOpeningScoreLimitTo(5000);
    expectEqual(config.getOpeningScoreLimit(), static_cast<uint32_t>(5000), "Opening Score should clamp to Winning Score");
    expectEqual(config.getBankThreshold(), static_cast<uint32_t>(5000), "compatibility getter should reflect the clamp");

    config.setOpeningScoreLimit(750);
    config.clampOpeningScoreLimitTo(5000);
    expectEqual(config.getOpeningScoreLimit(), static_cast<uint32_t>(750), "a valid Opening Score should remain unchanged");

    config.clampOpeningScoreLimitTo(500);
    expectEqual(config.getOpeningScoreLimit(), static_cast<uint32_t>(500), "lowering Winning Score should clamp Opening Score");
}

void gameManagerEnforcesSharedSetupInvariants()
{
    GameManager game;
    game.accessRuleConfig().setOpeningScoreLimit(6000);
    game.setPlayers({"A", "B"});
    game.setScoreLimit(5000);
    expectEqual(
        game.getRuleConfig().getOpeningScoreLimit(),
        static_cast<uint32_t>(5000),
        "the gameplay setup boundary should clamp Opening Score to Winning Score"
    );

    game.accessRuleConfig().setStraightEnabled(false);
    game.accessRuleConfig().setThreePairsEnabled(false);
    game.accessRuleConfig().setMultiplesEnabled(false);
    game.accessRuleConfig().setSinglesEnabled(false);
    bool rejected = false;
    try {
        game.startTurn(0);
    } catch (const std::logic_error&) {
        rejected = true;
    }
    expect(rejected, "the gameplay core should reject a turn with no scoring rule enabled");
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

void straightAndThreePairsScoreHotDice()
{
    GameManager straightGame;
    prepareTurn(straightGame, {"A", "B"});
    Dice& straightDice = setDice(straightGame, {{1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}, {6, 1}});

    const Checker straightChecker(straightGame);
    std::vector<Checker::Option> options = straightChecker.getAvailableOptions();
    expect(straightChecker.isStraight() && straightChecker.isStrait(), "Straight checker aliases should agree");
    expect(
        zilch::rules::RuleType::Straight == zilch::rules::RuleType::Strait,
        "Straight rule-type aliases should agree"
    );
    expect(hasOption(options, "Apply Straight (1000)"), "one-through-six should expose Straight scoring");
    runOption(options, "Apply Straight (1000)");

    expectEqual(straightGame.getCurrentPlayer()->getScore().getRoundScore(), static_cast<uint32_t>(1000), "straight should score 1000");
    expectEqual(straightDice.getNumDiceInPlay(), static_cast<uint16_t>(FULL_SET_OF_DICE), "scoring all dice should reset to hot dice");
    expect(straightDice.diceSetMap.empty(), "straight should consume all displayed dice");
    expect(straightGame.getSelectedOptionStatus(), "straight should mark an option as selected");

    GameManager setGame;
    prepareTurn(setGame, {"A", "B"});
    Dice& setDiceRef = setDice(setGame, {{1, 2}, {3, 2}, {6, 2}});

    const Checker setChecker(setGame);
    options = setChecker.getAvailableOptions();
    expect(setChecker.isThreePairs() && setChecker.isSet(), "Three Pairs checker aliases should agree");
    expect(
        zilch::rules::RuleType::ThreePairs == zilch::rules::RuleType::Set,
        "Three Pairs rule-type aliases should agree"
    );
    expect(hasOption(options, "Apply Three Pairs (1000)"), "three pairs should expose Three Pairs scoring");
    runOption(options, "Apply Three Pairs (1000)");

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
    expect(!hasOption(options, "Apply Straight (1000)"), "disabled Straight should not be offered");
    expect(hasOption(options, "Score single 1 (100)"), "disabled straights should not suppress singles");
    expect(hasOption(options, "Score single 5 (50)"), "disabled straights should not suppress singles");

    GameManager setGame;
    prepareTurn(setGame, {"A", "B"});
    setGame.accessRuleConfig().setSetsEnabled(false);
    setDice(setGame, {{1, 2}, {3, 2}, {6, 2}});
    const Checker setChecker(setGame);
    expect(!hasOption(setChecker.getAvailableOptions(), "Apply Three Pairs (1000)"), "disabled Three Pairs should not be offered");

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

void bankingRequiresRollSelectionScoreAndOpeningScore()
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
    game.accessRuleConfig().setOpeningScoreLimit(2000);
    expect(!game.canBankCurrentScore(), "banking should require meeting the Opening Score");

    score.increasePermanentScore(950);
    expect(!game.canBankCurrentScore(), "banking should still fail just below the Opening Score");
    score.increasePermanentScore(50);
    expect(game.canBankCurrentScore(), "banking should pass at the Opening Score");

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

void disabledStealingNeverCreatesOffer()
{
    GameManager game;
    prepareTurn(game, {"A", "B"});
    game.getCurrentPlayer()->getScore().increasePermanentScore(1000);
    game.getCurrentPlayer()->getScore().increaseRoundScore(100);
    game.getCurrentPlayer()->getDice().setNumDiceInPlay(4);
    game.registerRoll();
    game.setSelectedOptionStatus(true);
    game.endTurnAndBank();

    game.switchToNextPlayer();
    game.startTurn(game.getCurrentIndex());
    expect(!game.hasStealOfferForCurrentPlayer(), "disabled stealing should never create an offer");
}

void stealingCopiesBankedContinuationAndChains()
{
    GameManager game;
    prepareTurn(game, {"A", "B", "C"});
    game.accessRuleConfig().setStealingEnabled(true);
    for (Player& player : game.accessPlayers())
        player.getScore().increasePermanentScore(1000);

    Score& firstScore = game.getCurrentPlayer()->getScore();
    firstScore.increaseRoundScore(350);
    firstScore.setScoreFromMultiples(200);
    game.setValueOfChosenMultiple(2);
    game.getCurrentPlayer()->getDice().setNumDiceInPlay(3);
    game.registerRoll();
    game.setSelectedOptionStatus(true);
    game.endTurnAndBank();

    expectEqual(firstScore.getPermanentScore(), static_cast<uint32_t>(1350), "the first player should bank normally before offering a steal");

    game.switchToNextPlayer();
    game.startTurn(game.getCurrentIndex());
    expect(game.hasStealOfferForCurrentPlayer(), "the immediately following player should receive the steal offer");
    expect(game.canCurrentPlayerSteal(), "a player already on the board should be allowed to steal");
    expectEqual(game.getStealOfferDiceCount(), static_cast<uint16_t>(3), "the steal offer should preserve the unrolled dice count");
    expectEqual(game.getStealOfferScore(), static_cast<uint32_t>(350), "the steal offer should copy the prior turn score");
    expect(game.acceptStealOffer(), "an eligible player should be able to accept the steal offer");

    Score& secondScore = game.getCurrentPlayer()->getScore();
    expectEqual(secondScore.getRoundScore(), static_cast<uint32_t>(350), "accepting should copy the offered score into the new round");
    expectEqual(secondScore.getScoreFromMultiples(), static_cast<uint32_t>(200), "accepting should preserve an extendable multiple score");
    expectEqual(game.getValueOfChosenMultiple(), static_cast<uint16_t>(2), "accepting should preserve the saved multiple value");
    expectEqual(game.getCurrentPlayer()->getDice().getNumDiceInPlay(), static_cast<uint16_t>(3), "accepting should use only the offered dice");
    expect(game.isStealContinuationActive(), "accepting should mark the turn as a steal continuation");

    secondScore.increaseRoundScore(100);
    game.getCurrentPlayer()->getDice().setNumDiceInPlay(2);
    game.registerRoll();
    game.setSelectedOptionStatus(true);
    game.endTurnAndBank();
    expectEqual(secondScore.getPermanentScore(), static_cast<uint32_t>(1450), "the second player should bank inherited and newly scored points");

    game.switchToNextPlayer();
    game.startTurn(game.getCurrentIndex());
    expect(game.canCurrentPlayerSteal(), "a valid steal should chain to another on-board player");
    expectEqual(game.getStealOfferDiceCount(), static_cast<uint16_t>(2), "a chained steal should preserve the latest unrolled dice count");
    expectEqual(game.getStealOfferScore(), static_cast<uint32_t>(450), "a chained steal should carry the full continued score");
    expect(game.acceptStealOffer(), "the third player should be able to continue the chain");
    expectEqual(game.getCurrentPlayer()->getScore().getRoundScore(), static_cast<uint32_t>(450), "the chained score should become the third player's round score");
}

void stealingCannotPutPlayerOnBoardAndCanBeDeclined()
{
    GameManager game;
    prepareTurn(game, {"A", "B"});
    game.accessRuleConfig().setOpeningScoreLimit(750);
    game.accessRuleConfig().setStealingEnabled(true);
    game.accessPlayers()[0].getScore().increasePermanentScore(750);

    Score& score = game.getCurrentPlayer()->getScore();
    score.increaseRoundScore(250);
    game.getCurrentPlayer()->getDice().setNumDiceInPlay(4);
    game.registerRoll();
    game.setSelectedOptionStatus(true);
    game.endTurnAndBank();

    game.switchToNextPlayer();
    game.startTurn(game.getCurrentIndex());
    expect(game.hasStealOfferForCurrentPlayer(), "an off-board player should still be shown the preceding offer context");
    expect(!game.canCurrentPlayerSteal(), "a steal must not help a player reach the configured Opening Score");
    expect(!game.acceptStealOffer(), "an ineligible player must not be able to accept a steal");
    expectEqual(game.getCurrentPlayer()->getScore().getRoundScore(), static_cast<uint32_t>(0), "a rejected steal must not copy points");

    game.declineStealOffer();
    expect(!game.hasStealOfferForCurrentPlayer(), "starting fresh should clear the steal offer");
    expect(!game.isStealContinuationActive(), "starting fresh should leave no active steal chain");
    expectEqual(game.getCurrentPlayer()->getDice().getNumDiceInPlay(), static_cast<uint16_t>(FULL_SET_OF_DICE), "starting fresh should restore all six dice");
    expectEqual(game.getValueOfChosenMultiple(), static_cast<uint16_t>(0), "starting fresh should clear inherited combination state");
}

void stealingBustEndsChainWithoutFirstRollBonus()
{
    GameManager game;
    prepareTurn(game, {"A", "B"});
    game.accessRuleConfig().setStealingEnabled(true);
    game.accessPlayers()[0].getScore().increasePermanentScore(1000);
    game.accessPlayers()[1].getScore().increasePermanentScore(1000);

    game.getCurrentPlayer()->getScore().increaseRoundScore(300);
    game.getCurrentPlayer()->getDice().setNumDiceInPlay(3);
    game.registerRoll();
    game.setSelectedOptionStatus(true);
    game.endTurnAndBank();
    game.switchToNextPlayer();
    game.startTurn(game.getCurrentIndex());
    expect(game.acceptStealOffer(), "the second player should accept the prepared steal");

    setDice(game, {{2, 1}, {3, 1}, {4, 1}});
    game.registerRoll();
    const Checker checker(game);
    expect(!checker.isOptionAvailable(), "the stolen three-die hand should be a bust");
    checker.handleBusts();

    expectEqual(game.getCurrentPlayer()->getScore().getRoundScore(), static_cast<uint32_t>(0), "a steal bust should lose the inherited score");
    expect(game.getBustPending(), "a steal bust should require pass acknowledgment");
    expect(!game.getBustBonusUsedThisTurn(), "the first-roll bust bonus must not rescue a steal chain");
    expect(!game.isStealContinuationActive(), "a bust should end the steal chain");
}

void hotDiceEndsStealChainAndCannotBeOffered()
{
    GameManager game;
    prepareTurn(game, {"A", "B", "C"});
    game.accessRuleConfig().setStealingEnabled(true);
    for (Player& player : game.accessPlayers())
        player.getScore().increasePermanentScore(1000);

    game.getCurrentPlayer()->getScore().increaseRoundScore(300);
    game.getCurrentPlayer()->getDice().setNumDiceInPlay(3);
    game.registerRoll();
    game.setSelectedOptionStatus(true);
    game.endTurnAndBank();
    game.switchToNextPlayer();
    game.startTurn(game.getCurrentIndex());
    expect(game.acceptStealOffer(), "the second player should accept the prepared steal");

    setDice(game, {{1, 3}});
    game.registerRoll();
    const Checker checker(game);
    runOption(checker.getAvailableOptions(), "Score multiples of 1");
    expectEqual(game.getCurrentPlayer()->getDice().getNumDiceInPlay(), static_cast<uint16_t>(FULL_SET_OF_DICE), "scoring every stolen die should produce hot dice");
    expect(!game.isStealContinuationActive(), "using every unrolled die should end the steal chain");

    game.endTurnAndBank();
    game.switchToNextPlayer();
    game.startTurn(game.getCurrentIndex());
    expect(!game.hasStealOfferForCurrentPlayer(), "banking with hot dice should not create another steal offer");
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

void ruleConfigTogglesAndOpeningScoreAreStable()
{
    RuleConfig config;
    expect(!config.stealingEnabled(), "stealing should be disabled by default");

    config.toggleStraits();
    config.toggleSets();
    config.toggleMultiples();
    config.toggleSingles();
    config.toggleAnimations();
    config.toggleFirstRollBustBonus();
    config.toggleFinalChase();
    config.toggleAllowTies();
    config.toggleStealing();
    expect(!config.straitsEnabled(), "straits toggle should disable straights");
    expect(!config.setsEnabled(), "sets toggle should disable sets");
    expect(!config.multiplesEnabled(), "multiples toggle should disable multiples");
    expect(!config.singlesEnabled(), "singles toggle should disable singles");
    expect(!config.animationsEnabled(), "animations toggle should disable animations");
    expect(!config.firstRollBustBonusEnabled(), "first-roll bust toggle should disable the bonus");
    expect(!config.finalChaseEnabled(), "final chase toggle should disable final chase");
    expect(!config.tiesAllowed(), "allow ties toggle should disable the default");
    expect(config.stealingEnabled(), "stealing toggle should enable the optional variant");

    config.setStraitsEnabled(true);
    config.setSetsEnabled(true);
    config.setMultiplesEnabled(true);
    config.setSinglesEnabled(true);
    config.setFirstRollBustBonusEnabled(true);
    config.setFinalChaseEnabled(true);
    config.setAllowTies(false);
    config.setStealingEnabled(false);
    expect(config.straitsEnabled(), "straits setter should enable straights");
    expect(config.setsEnabled(), "sets setter should enable sets");
    expect(config.multiplesEnabled(), "multiples setter should enable multiples");
    expect(config.singlesEnabled(), "singles setter should enable singles");
    expect(config.firstRollBustBonusEnabled(), "first-roll bust setter should enable the bonus");
    expect(config.finalChaseEnabled(), "final chase setter should enable final chase");
    expect(!config.tiesAllowed(), "allow ties setter should disable ties");
    expect(!config.stealingEnabled(), "stealing setter should disable the optional variant");

    config.adjustOpeningScoreLimit(-2000);
    expectEqual(config.getOpeningScoreLimit(), static_cast<uint32_t>(0), "Opening Score should clamp at zero");
    config.adjustOpeningScoreLimit(250);
    expectEqual(config.getOpeningScoreLimit(), static_cast<uint32_t>(250), "Opening Score should increase from zero");
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
    runTest("mixedHumanAndComputerPlayerSetupIsPreserved", mixedHumanAndComputerPlayerSetupIsPreserved);
    runTest("computerPoliciesMatchTheTestedProfiles", computerPoliciesMatchTheTestedProfiles);
    runTest("easyScoresEveryAvailableDieBeforeRolling", easyScoresEveryAvailableDieBeforeRolling);
    runTest("easyBanksAtItsFixedThreshold", easyBanksAtItsFixedThreshold);
    runTest("hardCanPreserveAWeakScoringDieForTheReroll", hardCanPreserveAWeakScoringDieForTheReroll);
    runTest("hardSwitchesTheWholePolicyForStealing", hardSwitchesTheWholePolicyForStealing);
    runTest("hardBankThresholdKeepsFractionalPrecision", hardBankThresholdKeepsFractionalPrecision);
    runTest("hardRollsTheReported2800HotDiceState", hardRollsTheReported2800HotDiceState);
    runTest("hardCollectsGuaranteedPointsBeforeBanking", hardCollectsGuaranteedPointsBeforeBanking);
    runTest("hardBankCommitmentSurvivesHotDice", hardBankCommitmentSurvivesHotDice);
    runTest("hardCollectsBeforeImmediateWinAndFinalChaseBank", hardCollectsBeforeImmediateWinAndFinalChaseBank);
    runTest("computerBankCommitmentResetsAtTurnBoundaries", computerBankCommitmentResetsAtTurnBoundaries);
    runTest("stealingHardDoesNotUseBankCollector", stealingHardDoesNotUseBankCollector);
    runTest("mediumUsesOpponentAwareFinishDecisions", mediumUsesOpponentAwareFinishDecisions);
    runTest("computersTakeAReachableFinalChaseWin", computersTakeAReachableFinalChaseWin);
    runTest("hardUsesTheStealingSpecificAcceptanceCurve", hardUsesTheStealingSpecificAcceptanceCurve);
    runTest("computerActionPlannerCoversTurnBoundaries", computerActionPlannerCoversTurnBoundaries);
    runTest("sharedDefaultsAndOpeningScoreAliasesAreStable", sharedDefaultsAndOpeningScoreAliasesAreStable);
    runTest("allDisabledScoringRulesAreDetected", allDisabledScoringRulesAreDetected);
    runTest("openingScoreClampsToWinningScore", openingScoreClampsToWinningScore);
    runTest("gameManagerEnforcesSharedSetupInvariants", gameManagerEnforcesSharedSetupInvariants);
    runTest("gameStateMachineDescribesEveryState", gameStateMachineDescribesEveryState);
    runTest("singlesDoNotMasqueradeAsMultiples", singlesDoNotMasqueradeAsMultiples);
    runTest("singlesScoreOnlyOneDie", singlesScoreOnlyOneDie);
    runTest("straightAndThreePairsScoreHotDice", straightAndThreePairsScoreHotDice);
    runTest("multiplesScoreScaleAndCanBeExtended", multiplesScoreScaleAndCanBeExtended);
    runTest("fourOnesDoubleBaseMultipleAndHotDice", fourOnesDoubleBaseMultipleAndHotDice);
    runTest("ruleManagerExposesStructuredGameOptions", ruleManagerExposesStructuredGameOptions);
    runTest("ruleTogglesSuppressOnlyTheirConfiguredOptions", ruleTogglesSuppressOnlyTheirConfiguredOptions);
    runTest("firstRollBustBonusIsOneTimeAndRestartsRoll", firstRollBustBonusIsOneTimeAndRestartsRoll);
    runTest("disabledBustBonusBustsImmediately", disabledBustBonusBustsImmediately);
    runTest("nonBustHandsDoNotTriggerBustHandling", nonBustHandsDoNotTriggerBustHandling);
    runTest("emptyDiceAfterScoringResetsToHotDice", emptyDiceAfterScoringResetsToHotDice);
    runTest("bankingRequiresRollSelectionScoreAndOpeningScore", bankingRequiresRollSelectionScoreAndOpeningScore);
    runTest("endTurnAndBankMovesRoundScoreOnlyWhenAllowed", endTurnAndBankMovesRoundScoreOnlyWhenAllowed);
    runTest("disabledStealingNeverCreatesOffer", disabledStealingNeverCreatesOffer);
    runTest("stealingCopiesBankedContinuationAndChains", stealingCopiesBankedContinuationAndChains);
    runTest("stealingCannotPutPlayerOnBoardAndCanBeDeclined", stealingCannotPutPlayerOnBoardAndCanBeDeclined);
    runTest("stealingBustEndsChainWithoutFirstRollBonus", stealingBustEndsChainWithoutFirstRollBonus);
    runTest("hotDiceEndsStealChainAndCannotBeOffered", hotDiceEndsStealChainAndCannotBeOffered);
    runTest("finalRoundRequiresBeatingLeaderWhenTiesDisabled", finalRoundRequiresBeatingLeaderWhenTiesDisabled);
    runTest("turnOrderFinalRoundAndResetStateWrapSafely", turnOrderFinalRoundAndResetStateWrapSafely);
    runTest("selectionAndSavedMultipleStateResetCorrectly", selectionAndSavedMultipleStateResetCorrectly);
    runTest("ruleConfigTogglesAndOpeningScoreAreStable", ruleConfigTogglesAndOpeningScoreAreStable);
    runTest("winnerLookupHandlesEmptyAndHighestScore", winnerLookupHandlesEmptyAndHighestScore);
    runTest("resourcePathLeavesAbsoluteAndMissingRelativePathsStable", resourcePathLeavesAbsoluteAndMissingRelativePathsStable);
    runTest("emptyPlayerResetIsSafe", emptyPlayerResetIsSafe);

    std::cout << "All rule tests passed.\n";
    return EXIT_SUCCESS;
}
