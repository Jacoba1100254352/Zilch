/**
Author: Jacob Anderson
Class: ECE-6122-A
Last Date Modified: 12/2/2025

Description:
This file contains the implementation of the Checker class, which adapts the
rule-manager option model to the existing UI-facing scoring API.
*/

#include "Checker.h"

#include "Dice.h"
#include "GameManager.h"
#include "Player.h"
#include "Score.h"
#include "rules/RuleManager.h"

namespace {
[[nodiscard]] zilch::rules::RuleManager makeRuleManager(const GameManager& game)
{
    return zilch::rules::RuleManager(game.getRuleConfig());
}
}

std::vector<Checker::Option> Checker::getAvailableOptions() const
{
    const std::vector<zilch::rules::GameOption> gameOptions = makeRuleManager(game).evaluate(game);
    std::vector<Option> options;
    options.reserve(gameOptions.size());

    for (const zilch::rules::GameOption& gameOption : gameOptions) {
        options.push_back({
            gameOption.label,
            [this, gameOption]() {
                makeRuleManager(game).apply(game, gameOption);
            }
        });
    }

    return options;
}

void Checker::handleBusts() const
{
    Player* player = game.getCurrentPlayer();
    if (!player)
        return;

    const Dice& dice = player->getDice();
    Score& score = player->getScore();

    if (dice.getNumDiceInPlay() >= 1 && isOptionAvailable())
        return;

    // Optional first-roll bust bonus: award 50 points and restart dice if enabled and unused.
    if (game.getRuleConfig().firstRollBustBonusEnabled() &&
        dice.getNumDiceInPlay() >= 1 &&
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

    if (dice.getNumDiceInPlay() >= 1) {
        score.setRoundScore(0);
        game.setTurnContinuationStatus(false, true);
    } else {
        game.manageDiceCount(FULL_SET_OF_DICE);
    }
}

bool Checker::isStrait() const
{
    return makeRuleManager(game).hasOption(game, zilch::rules::RuleType::Strait);
}

bool Checker::isSet() const
{
    return makeRuleManager(game).hasOption(game, zilch::rules::RuleType::Set);
}

bool Checker::isMultiple() const
{
    return makeRuleManager(game).hasOption(game, zilch::rules::RuleType::Multiple);
}

bool Checker::isDesiredMultipleAvailable(const uint16_t desiredMultiple) const
{
    return makeRuleManager(game).hasOption(game, zilch::rules::RuleType::Multiple, desiredMultiple);
}

bool Checker::canAddMultiples() const
{
    return makeRuleManager(game).hasOption(game, zilch::rules::RuleType::AddMultiple, game.getValueOfChosenMultiple());
}

bool Checker::isSingle(const uint16_t single) const
{
    return makeRuleManager(game).hasOption(game, zilch::rules::RuleType::Single, single);
}

bool Checker::isOptionAvailable() const
{
    return !makeRuleManager(game).evaluate(game).empty();
}
