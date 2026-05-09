/**
Author: Jacob Anderson
Class: ECE-6122-A
Last Date Modified: 12/2/2025

Description:
This file contains the implementation of the Checker class, which is responsible for evaluating
the current state of the dice in the game and determining available scoring options, handling busts,
and applying scoring rules based on the game's configuration.
*/

#include "Checker.h"

#include <algorithm>
#include <cmath>
#include <ranges>

#include "Dice.h"
#include "GameManager.h"
#include "Player.h"
#include "Score.h"

/**
 * @brief Removes dice of the specified value and updates the game state accordingly.
 *
 * @param game Reference to the GameManager instance.
 * @param dieValue The value of the dice to remove.
 */
static void removeDiceAndUpdate(GameManager& game, const uint16_t dieValue)
{
    Dice& dice = game.getCurrentPlayer()->getDice();
    game.manageDiceCount(dice.getNumDiceInPlay() - dice.diceSetMap[dieValue]);
    dice.eliminateDice(dieValue);
    dice.calculateMultipleAvailability();
    game.setSelectedOptionStatus(true);
}

/**
 * @brief Retrieves the available scoring options based on the current dice state.
 *
 * @return A vector of available options with their corresponding actions.
 */
std::vector<Checker::Option> Checker::getAvailableOptions() const
{
    std::vector<Option> options;
    // Add straight and set options first.
    if (isStrait()) {
        options.push_back({"Apply strait (1000)", [this]() { applyStrait(); }});
    }
    if (isSet()) {
        options.push_back({"Apply set (3 pairs)", [this]() { applySet(); }});
    }

    // Add multiple-based scoring options.
    for (uint16_t dieValue = 1; dieValue <= FULL_SET_OF_DICE; ++dieValue) {
        if ((isMultiple() && isDesiredMultipleAvailable(dieValue)) ||
            ((game.getCurrentPlayer()->getScore().getScoreFromMultiples() >= 200) &&
                (game.getValueOfChosenMultiple() == dieValue) && canAddMultiples())) {
            options.push_back({"Score multiples of " + std::to_string(dieValue), [this, dieValue]() { applyMultiple(dieValue); }});
        }
    }

    // Add single scoring options.
    if (isSingle(1))
        options.push_back({"Score single 1 (100)", [this]() { applySingle(1); }});
    if (isSingle(5))
        options.push_back({"Score single 5 (50)", [this]() { applySingle(5); }});

    return options;
}

/**
 * @brief Handles bust scenarios based on the current dice state.
 */
void Checker::handleBusts() const
{
    Player* player = game.getCurrentPlayer();
    const Dice& dice = player->getDice();
    Score& score = player->getScore();

    // Optional first-roll bust bonus: award 50 points and restart dice if enabled and unused.
    if (game.getRuleConfig().firstRollBustBonusEnabled() &&
        game.getRollCountThisTurn() == 1 &&
        !game.getBustBonusUsedThisTurn()) {
        score.increaseRoundScore(50);
        game.manageDiceCount(FULL_SET_OF_DICE);
        game.setBustBonusUsedThisTurn(true);
        game.setTurnContinuationStatus(true, false);
        game.setHasRolledThisTurn(false);
        game.setSelectedOptionStatus(false);
        return;
    }

    // Handle bust logic based on remaining dice and current round score.
    if (game.getRuleConfig().firstRollBustBonusEnabled() &&
        dice.getNumDiceInPlay() == FULL_SET_OF_DICE && !score.getRoundScore()) {
        score.increaseRoundScore(50);
        game.setTurnContinuationStatus(true, true);
    } else if (dice.getNumDiceInPlay() >= 1) {
        score.setRoundScore(0);
        game.setTurnContinuationStatus(false, true);
    } else {
        game.manageDiceCount(FULL_SET_OF_DICE);
    }
}

/**
 * @brief Checks if the current dice form a strait (1-6).
 *
 * @return True if a strait is present, false otherwise.
 */
bool Checker::isStrait() const
{
    if (!game.getRuleConfig().straitsEnabled())
        return false;
    std::map<uint16_t, uint16_t>& diceSetMap = game.getCurrentPlayer()->getDice().diceSetMap;
    uint16_t i = 0;
    return (diceSetMap.size() == FULL_SET_OF_DICE && std::ranges::all_of(
        diceSetMap, [&i](const auto& die) {
            return die.first == ++i && die.second == 1;
        }
    ));
}

/**
 * @brief Checks if the current dice form a set (three pairs).
 *
 * @return True if a set is present, false otherwise.
 */
bool Checker::isSet() const
{
    if (!game.getRuleConfig().setsEnabled())
        return false;
    return (game.getCurrentPlayer()->getDice().diceSetMap.size() == 3 && !isStrait() && !isMultiple() &&
        std::ranges::all_of(
            game.getCurrentPlayer()->getDice().diceSetMap,
            [](const auto& die) { return die.second == 2; }
        ));
}

/**
 * @brief Checks if any multiples (three or more of a kind) are available.
 *
 * @return True if multiples are available, false otherwise.
 */
bool Checker::isMultiple() const
{
    if (!game.getRuleConfig().multiplesEnabled())
        return false;
    return game.getCurrentPlayer()->getDice().isMultipleAvailable();
}

/**
 * @brief Checks if the desired multiple is available.
 *
 * @param desiredMultiple The value of the desired multiple.
 * @return True if the desired multiple is available, false otherwise.
 */
bool Checker::isDesiredMultipleAvailable(const uint16_t desiredMultiple) const
{
    const auto& diceMap = game.getCurrentPlayer()->getDice().diceSetMap;
    if (const auto it = diceMap.find(desiredMultiple); it != diceMap.end())
        return it->second >= 3;
    return false;
}

/**
 * @brief Checks if additional multiples can be added to the current scoring.
 *
 * @return True if additional multiples can be added, false otherwise.
 */
bool Checker::canAddMultiples() const
{
    const auto& diceMap = game.getCurrentPlayer()->getDice().diceSetMap;
    if (const auto it = diceMap.find(game.getValueOfChosenMultiple()); it != diceMap.end())
        return it->second > 0;
    return false;
}

/**
 * @brief Checks if a single die of the specified value is available for scoring.
 *
 * @param single The value of the single die to check (1 or 5).
 * @return True if the single die is available, false otherwise.
 */
bool Checker::isSingle(uint16_t single) const
{
    if (!game.getRuleConfig().singlesEnabled())
        return false;
    return ((single == 1 || single == 5) && std::ranges::any_of(
        game.getCurrentPlayer()->getDice().diceSetMap,
        [single](const auto& die) {
            return die.first == single;
        }
    ));
}

/**
 * @brief Checks if any scoring options are available based on the current dice state.
 *
 * @return True if at least one option is available, false otherwise.
 */
bool Checker::isOptionAvailable() const
{
    return (isStrait() || isSet() || isMultiple() || canAddMultiples() || isSingle(1) || isSingle(5));
}

/**
 * @brief Applies the strait scoring option.
 */
void Checker::applyStrait() const
{
    game.getCurrentPlayer()->getScore().increaseRoundScore(1000);
    game.getCurrentPlayer()->getDice().diceSetMap.clear();
    game.setSelectedOptionStatus(true);
    game.manageDiceCount(0);
}

/**
 * @brief Applies the set scoring option.
 */
void Checker::applySet() const
{
    game.getCurrentPlayer()->getScore().increaseRoundScore(1000);
    game.getCurrentPlayer()->getDice().diceSetMap.clear();
    game.setSelectedOptionStatus(true);
    game.manageDiceCount(0);
}

/**
 * @brief Applies the multiple scoring option for the specified die value.
 *
 * @param dieValue The value of the die to apply multiples for.
 */
void Checker::applyMultiple(const uint16_t dieValue) const
{
    Player* const& currentPlayer = game.getCurrentPlayer();
    Dice& dice = currentPlayer->getDice();
    Score& score = currentPlayer->getScore();

    // Score the base multiple or extend an existing multiple chain.
    if (isMultiple() && dice.diceSetMap[dieValue] >= 3) {
        const uint16_t baseScore = (dieValue == 1) ? 1000 : dieValue * 100;
        const uint16_t numMultiples = dice.diceSetMap[dieValue] - 3;
        const auto mScore = static_cast<uint16_t>(std::pow(2, numMultiples) * baseScore);
        score.setScoreFromMultiples(mScore);
        score.increaseRoundScore(mScore);
        game.setValueOfChosenMultiple(dieValue);
    } else if (canAddMultiples()) {
        const auto mScore = static_cast<uint16_t>(std::pow(2, dice.diceSetMap[dieValue]) * score.getScoreFromMultiples());
        score.increaseRoundScore(mScore - score.getScoreFromMultiples());
        score.setScoreFromMultiples(mScore);
    } else return;

    // Remove scored dice and refresh availability.
    removeDiceAndUpdate(game, dieValue);
}

/**
 * @brief Applies the single die scoring option for the specified die value.
 *
 * @param dieValue The value of the single die to apply (1 or 5).
 */
void Checker::applySingle(const uint16_t dieValue) const
{
    Player* currentPlayer = game.getCurrentPlayer();
    Score& score = currentPlayer->getScore();

    // Decide whether to treat the single as part of an existing multiple or a standalone score.
    if (std::map<uint16_t, uint16_t>& diceSetMap = currentPlayer->getDice().diceSetMap; (game.getValueOfChosenMultiple() == dieValue && canAddMultiples()) ||
        (diceSetMap[dieValue] >= 3 && isMultiple())) {
        applyMultiple(dieValue);
    } else if (1 <= diceSetMap[dieValue] && diceSetMap[dieValue] < 3) {
        score.increaseRoundScore((dieValue == 1) ? 100 : 50);
        diceSetMap[dieValue]--;
        game.manageDiceCount(currentPlayer->getDice().getNumDiceInPlay() - 1);
        game.setSelectedOptionStatus(true);
    }
}
