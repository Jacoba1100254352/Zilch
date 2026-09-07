#include "ComputerStrategy.h"
#include "GameManager.h"
#include "rules/RuleManager.h"

#include <array>
#include <functional>
#include <iostream>
#include <stdexcept>

namespace {

using zilch::computer::TurnAction;

void expect(const bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

GameManager checkpoint(const std::uint32_t risk, const std::uint32_t own, const std::uint32_t opponent,
                       const std::initializer_list<std::uint16_t> roll,
                       const std::uint32_t savedScore = 0, const std::uint16_t savedFace = 6,
                       const std::uint32_t target = 10000, const bool finalActive = false)
{
    GameManager game;
    game.setPlayerSetups({{"Computer", PlayerKind::Computer, ComputerDifficulty::Hard},
                         {"Opponent", PlayerKind::Human, ComputerDifficulty::Medium}});
    game.setScoreLimit(target);
    game.accessPlayers()[0].getScore().increasePermanentScore(own);
    game.accessPlayers()[1].getScore().increasePermanentScore(opponent);
    if (finalActive) {
        game.startTurn(1);
        game.startFinalRound();
    }
    game.startTurn(0);
    game.getCurrentPlayer()->getScore().setRoundScore(risk);
    game.manageDiceCount(static_cast<std::uint16_t>(roll.size()));
    auto& dice = game.getCurrentPlayer()->getDice();
    for (const auto face : roll)
        ++dice.diceSetMap[face];
    dice.calculateMultipleAvailability();
    if (savedScore != 0) {
        game.setValueOfChosenMultiple(savedFace);
        game.getCurrentPlayer()->getScore().setScoreFromMultiples(savedScore);
    }
    game.registerRoll();
    return game;
}

struct Result {
    GameManager game;
    TurnAction action;
};

Result select(GameManager game)
{
    const zilch::rules::RuleManager scorer(game.getRuleConfig());
    for (unsigned int step = 0; step <= 6; ++step) {
        const auto options = scorer.evaluate(game);
        const auto action = zilch::computer::chooseTurnAction(game, options.empty(), !options.empty(), options);
        if (action == TurnAction::SelectOption) {
            const auto index = zilch::computer::chooseOption(game, options);
            expect(index < options.size(), "A committed plan must reference the current legal options.");
            scorer.apply(game, options[index]);
        } else {
            expect(action == TurnAction::Roll || action == TurnAction::Bank, "Unexpected terminal scoring action.");
            return {game, action};
        }
    }
    throw std::runtime_error("Selection must terminate within six scoring dice.");
}

void threeAxesAndOpening()
{
    const auto low = select(checkpoint(0, 1000, 1000, {6, 6, 6, 5, 2, 3}));
    expect(low.action == TurnAction::Roll && low.game.getCurrentPlayer()->getScore().getRoundScore() == 600 &&
               low.game.getCurrentPlayer()->getDice().getNumDiceInPlay() == 3,
           "Original 666+5 checkpoint must keep the sixes and reroll three dice.");
    const auto stronger = select(checkpoint(2000, 1000, 1000, {6, 6, 6, 5, 2, 3}));
    expect(stronger.action == TurnAction::Roll && stronger.game.getCurrentPlayer()->getScore().getRoundScore() == 2600,
           "A valuable six chain can justify continuing with 2600 at risk.");
    const auto expensive = select(checkpoint(3500, 1000, 1000, {6, 6, 6, 5, 2, 3}));
    expect(expensive.action == TurnAction::Bank && expensive.game.getCurrentPlayer()->getScore().getRoundScore() == 4150,
           "Higher points at risk must allow banking the multiple and extra five.");
    const auto weaker = select(checkpoint(2000, 1000, 1000, {2, 2, 2, 5, 3, 4}));
    expect(weaker.action == TurnAction::Bank && weaker.game.getCurrentPlayer()->getScore().getRoundScore() == 2250,
           "The weaker two chain should bank where the six chain keeps rolling.");
    const auto farOpponent = select(checkpoint(900, 3500, 1000, {6, 6, 6, 5, 2, 3}, 0, 6, 5000));
    const auto closeOpponent = select(checkpoint(900, 3500, 4500, {6, 6, 6, 5, 2, 3}, 0, 6, 5000));
    expect(farOpponent.action == TurnAction::Bank && closeOpponent.action == TurnAction::Roll,
           "Opponent position must change whether to bank the target or seek a safer finishing buffer.");
    auto unopened = checkpoint(0, 0, 0, {6, 6, 6, 5, 2, 3});
    unopened.accessRuleConfig().setOpeningScoreLimit(3000);
    const auto opening = select(unopened);
    expect(opening.action == TurnAction::Roll && !opening.game.canBankCurrentScore(),
           "No candidate may bank before the configured opening minimum.");
}

void scorerExtensionsAndHotDice()
{
    for (std::uint16_t face = 1; face <= 6; ++face) {
        const auto base = face == 1 ? 1000U : static_cast<unsigned int>(face * 100);
        auto game = checkpoint(base, 1000, 1000, {face, face, face}, base, face);
        const zilch::rules::RuleManager scorer(game.getRuleConfig());
        const auto options = scorer.evaluate(game);
        expect(options.size() == 1 && options.front().ruleType == zilch::rules::RuleType::AddMultiple &&
                   options.front().pointsAwarded == base * 7,
               "All three matching remaining dice must extend, not replace, the saved multiple.");
        scorer.apply(game, options.front());
        expect(game.getCurrentPlayer()->getScore().getRoundScore() == base * 8 &&
                   game.getCurrentPlayer()->getDice().getNumDiceInPlay() == 6 &&
                   game.getValueOfChosenMultiple() == 0 &&
                   game.getCurrentPlayer()->getScore().getScoreFromMultiples() == 0,
               "A six-of-a-kind extension must score fully and immediately clear the hot-dice chain.");
    }
    const auto hot = select(checkpoint(600, 1000, 1000, {6, 1, 5}, 600));
    expect(hot.action == TurnAction::Roll && hot.game.getCurrentPlayer()->getScore().getRoundScore() == 1350 &&
               hot.game.getCurrentPlayer()->getDice().getNumDiceInPlay() == 6 &&
               hot.game.getValueOfChosenMultiple() == 0,
           "The joint extension-plus-singles path must survive hot dice and the UI waiting state.");
    auto bankHot = checkpoint(4800, 0, 0, {6, 1, 5}, 600, 6, 5000);
    bankHot.accessRuleConfig().setFinalChaseEnabled(false);
    const auto bank = select(bankHot);
    expect(bank.action == TurnAction::Bank && bank.game.getCurrentPlayer()->getDice().getNumDiceInPlay() == 6,
           "A committed winning bank must survive scoring the final hot-dice option.");
}

void safeFinishesAndOrdinaryScope()
{
    auto final = checkpoint(0, 4900, 5500, {6, 6, 6, 5, 2, 3}, 0, 6, 5000, true);
    final.accessRuleConfig().setAllowTies(false);
    const auto win = select(final);
    expect(win.action == TurnAction::Bank && win.game.getCurrentPlayer()->getScore().getRoundScore() == 650,
           "Collect the five as well as the sixes to secure the final-chase win.");
    auto tie = checkpoint(0, 4850, 5500, {6, 6, 6, 5, 2, 3}, 0, 6, 5000, true);
    tie.accessRuleConfig().setAllowTies(false);
    expect(select(tie).action == TurnAction::Roll, "A forbidden final tie must not count as an outright win.");
    auto singleWin = checkpoint(0, 4900, 5050, {1, 1, 2, 3, 4, 6}, 0, 6, 5000, true);
    singleWin.accessRuleConfig().setAllowTies(false);
    const auto singles = select(singleWin);
    expect(singles.action == TurnAction::Bank && singles.game.getCurrentPlayer()->getScore().getRoundScore() == 200,
           "Safe-finish protection also applies to an ordinary non-chain roll.");
    const auto ordinary = select(checkpoint(4850, 0, 4400, {1, 1, 2, 3, 4, 6}, 0, 6, 5000));
    expect(ordinary.action == TurnAction::Bank && ordinary.game.getCurrentPlayer()->getScore().getRoundScore() == 5050 &&
               !ordinary.game.getComputerJointBankDecision().has_value(),
           "Non-chain rolls keep the ordinary greedy collector, not full-joint stop-short selection.");
}

void copiesAndPlanningAreIsolated()
{
    auto original = checkpoint(600, 1000, 5500, {6, 1, 5}, 600, 6, 5000, true);
    original.setComputerJointPlan({0, 1}, false);
    original.setComputerSafeFinishCommitted(true);
    auto copy = original;
    expect(copy.getCurrentPlayer() == &copy.accessPlayers()[copy.getCurrentIndex()] &&
               copy.getCurrentPlayer() != original.getCurrentPlayer(), "A copied game must own its active player.");
    expect(copy.isFinalRoundActive() && copy.getFinalRoundStarterIndex() == original.getFinalRoundStarterIndex() &&
               copy.getRollCountThisTurn() == original.getRollCountThisTurn() &&
               copy.getComputerJointSelections() == original.getComputerJointSelections(),
           "Copying must preserve game position, roll state, and private plan state.");
    copy.getCurrentPlayer()->getScore().setRoundScore(999);
    copy.getCurrentPlayer()->getDice().diceSetMap.clear();
    copy.clearComputerIntent();
    expect(original.getCurrentPlayer()->getScore().getRoundScore() == 600 &&
               original.getCurrentPlayer()->getDice().diceSetMap.size() == 3 &&
               original.getComputerJointSelections().size() == 2, "A copied branch must not mutate its source.");
    GameManager assigned;
    assigned = original;
    assigned.getCurrentPlayer()->getScore().setRoundScore(100);
    expect(assigned.getCurrentPlayer() == &assigned.accessPlayers()[assigned.getCurrentIndex()] &&
               original.getCurrentPlayer()->getScore().getRoundScore() == 600,
           "Copy assignment must rebind the current player too.");
    GameManager empty;
    assigned = empty;
    expect(!assigned.getCurrentPlayer() && assigned.getPlayers().empty(), "Empty-game assignment must clear the player pointer.");

    auto game = checkpoint(600, 1000, 1000, {6, 1, 5}, 600);
    const auto diceBefore = game.getCurrentPlayer()->getDice().diceSetMap;
    const zilch::rules::RuleManager scorer(game.getRuleConfig());
    const auto choice = zilch::computer::chooseOption(game, scorer.evaluate(game));
    expect(choice < scorer.evaluate(game).size() && game.getCurrentPlayer()->getScore().getRoundScore() == 600 &&
               game.getCurrentPlayer()->getDice().diceSetMap == diceBefore &&
               game.getCurrentPlayer()->getScore().getScoreFromMultiples() == 600,
           "Planning may store intent but cannot score or consume any live dice.");
}

void intentBoundaries()
{
    const std::vector<std::function<void(GameManager&)>> boundaries{
        [](GameManager& game) { game.registerRoll(); },
        [](GameManager& game) { game.startTurn(0); },
        [](GameManager& game) { game.switchToNextPlayer(); },
        [](GameManager& game) { game.setTurnContinuationStatus(false, true); },
        [](GameManager& game) { game.setTurnContinuationStatus(false); },
        [](GameManager& game) { game.setPlayers({"New game"}); },
        [](GameManager& game) { game.endTurnAndBank(); }
    };
    for (const auto& boundary : boundaries) {
        auto game = checkpoint(1400, 1000, 1000, {6, 1, 5}, 600);
        game.setSelectedOptionStatus(true);
        game.setComputerJointPlan({0, 1}, true);
        game.setComputerBankCommitted(true);
        game.setComputerSafeFinishCommitted(true);
        boundary(game);
        expect(!game.getComputerJointBankDecision().has_value() && game.getComputerJointSelections().empty() &&
                   !game.getComputerBankCommitted() && !game.getComputerSafeFinishCommitted(),
               "Roll, turn, player, bust, new-game and bank boundaries must clear every computer intent field.");
    }
}

} // namespace

int main()
{
    try {
        threeAxesAndOpening();
        scorerExtensionsAndHotDice();
        safeFinishesAndOrdinaryScope();
        copiesAndPlanningAreIsolated();
        intentBoundaries();
        std::cout << "Visual chain strategy, scorer, copied-state and paced-action tests passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
