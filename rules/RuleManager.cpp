#include "rules/RuleManager.h"

#include <algorithm>
#include <cmath>
#include <ranges>

#include "Dice.h"
#include "GameManager.h"
#include "Player.h"
#include "Score.h"

namespace zilch::rules {
namespace {
uint32_t baseMultipleScore(const uint16_t dieValue)
{
    return dieValue == 1 ? 1000U : static_cast<uint32_t>(dieValue) * 100U;
}

uint32_t multipleScore(const uint16_t dieValue, const uint16_t count)
{
    if (count < 3)
        return 0;
    return static_cast<uint32_t>(std::pow(2, count - 3)) * baseMultipleScore(dieValue);
}

std::map<uint16_t, uint16_t> fullDiceSet(const Dice& dice)
{
    return dice.diceSetMap;
}

class StraitRule final : public IRule {
public:
    [[nodiscard]] RuleType type() const override { return RuleType::Strait; }

    [[nodiscard]] std::vector<GameOption> evaluate(const RuleContext& context) const override
    {
        if (!context.config.straitsEnabled())
            return {};

        uint16_t expected = 0;
        const bool isStrait = context.dice.diceSetMap.size() == FULL_SET_OF_DICE && std::ranges::all_of(
            context.dice.diceSetMap,
            [&expected](const auto& die) {
                return die.first == ++expected && die.second == 1;
            }
        );

        if (!isStrait)
            return {};

        return {{RuleType::Strait, "Apply strait (1000)", 0, 1000, 0, fullDiceSet(context.dice)}};
    }
};

class SetRule final : public IRule {
public:
    [[nodiscard]] RuleType type() const override { return RuleType::Set; }

    [[nodiscard]] std::vector<GameOption> evaluate(const RuleContext& context) const override
    {
        if (!context.config.setsEnabled())
            return {};

        const bool isSet = context.dice.diceSetMap.size() == 3 && std::ranges::all_of(
            context.dice.diceSetMap,
            [](const auto& die) { return die.second == 2; }
        );

        if (!isSet)
            return {};

        return {{RuleType::Set, "Apply set (3 pairs)", 0, 1000, 0, fullDiceSet(context.dice)}};
    }
};

class MultipleRule final : public IRule {
public:
    [[nodiscard]] RuleType type() const override { return RuleType::Multiple; }

    [[nodiscard]] std::vector<GameOption> evaluate(const RuleContext& context) const override
    {
        if (!context.config.multiplesEnabled())
            return {};

        std::vector<GameOption> options;
        for (const auto& [dieValue, count] : context.dice.diceSetMap) {
            if (count < 3)
                continue;

            const uint32_t score = multipleScore(dieValue, count);
            options.push_back({
                RuleType::Multiple,
                "Score multiples of " + std::to_string(dieValue),
                dieValue,
                score,
                score,
                {{dieValue, count}}
            });
        }
        return options;
    }
};

class AddMultipleRule final : public IRule {
public:
    [[nodiscard]] RuleType type() const override { return RuleType::AddMultiple; }

    [[nodiscard]] std::vector<GameOption> evaluate(const RuleContext& context) const override
    {
        if (!context.config.multiplesEnabled() || context.game.getValueOfChosenMultiple() == 0 ||
            context.score.getScoreFromMultiples() < 200)
            return {};

        const uint16_t dieValue = context.game.getValueOfChosenMultiple();
        const auto iter = context.dice.diceSetMap.find(dieValue);
        if (iter == context.dice.diceSetMap.end() || iter->second == 0 || iter->second >= 3)
            return {};

        const uint32_t previousScore = context.score.getScoreFromMultiples();
        const auto scale = static_cast<uint32_t>(std::pow(2, iter->second));
        const uint32_t updatedScore = previousScore * scale;
        return {{
            RuleType::AddMultiple,
            "Score multiples of " + std::to_string(dieValue),
            dieValue,
            updatedScore - previousScore,
            updatedScore,
            {{dieValue, iter->second}}
        }};
    }
};

class SingleRule final : public IRule {
public:
    [[nodiscard]] RuleType type() const override { return RuleType::Single; }

    [[nodiscard]] std::vector<GameOption> evaluate(const RuleContext& context) const override
    {
        if (!context.config.singlesEnabled())
            return {};

        std::vector<GameOption> options;
        for (const uint16_t dieValue : {static_cast<uint16_t>(1), static_cast<uint16_t>(5)}) {
            const auto iter = context.dice.diceSetMap.find(dieValue);
            if (iter == context.dice.diceSetMap.end() || iter->second == 0)
                continue;

            if (context.config.multiplesEnabled()) {
                if (context.game.getValueOfChosenMultiple() == dieValue && context.score.getScoreFromMultiples() >= 200)
                    continue;
                if (context.dice.isMultipleAvailable() && iter->second >= 3)
                    continue;
            }

            options.push_back({
                RuleType::Single,
                dieValue == 1 ? "Score single 1 (100)" : "Score single 5 (50)",
                dieValue,
                dieValue == 1 ? 100U : 50U,
                0,
                {{dieValue, 1}}
            });
        }
        return options;
    }
};

RuleContext makeContext(const GameManager& game, const RuleConfig& config)
{
    const Player& player = *game.getCurrentPlayer();
    return {game, player, player.getDice(), player.getScore(), config};
}

void consumeDice(GameManager& game, const GameOption& option)
{
    Player* player = game.getCurrentPlayer();
    if (!player)
        return;

    Dice& dice = player->getDice();
    uint16_t consumedCount = 0;
    for (const auto& [dieValue, count] : option.consumedDice) {
        const auto iter = dice.diceSetMap.find(dieValue);
        if (iter == dice.diceSetMap.end())
            continue;

        consumedCount = static_cast<uint16_t>(consumedCount + std::min(count, iter->second));
        if (count >= iter->second)
            dice.diceSetMap.erase(iter);
        else
            iter->second = static_cast<uint16_t>(iter->second - count);
    }

    const uint16_t currentCount = dice.getNumDiceInPlay();
    const uint16_t remainingCount = consumedCount >= currentCount ? 0 : static_cast<uint16_t>(currentCount - consumedCount);
    game.manageDiceCount(remainingCount);
    dice.calculateMultipleAvailability();
    game.setSelectedOptionStatus(true);
}
}

RuleManager::RuleManager(const RuleConfig& config)
    : config(config)
{
    rules.push_back(std::make_unique<StraitRule>());
    rules.push_back(std::make_unique<SetRule>());
    rules.push_back(std::make_unique<MultipleRule>());
    rules.push_back(std::make_unique<AddMultipleRule>());
    rules.push_back(std::make_unique<SingleRule>());
}

std::vector<GameOption> RuleManager::evaluate(const GameManager& game) const
{
    if (!game.getCurrentPlayer())
        return {};

    const RuleContext context = makeContext(game, config);
    std::vector<GameOption> options;
    for (const auto& rule : rules) {
        std::vector<GameOption> ruleOptions = rule->evaluate(context);
        options.insert(options.end(), ruleOptions.begin(), ruleOptions.end());
    }
    return options;
}

bool RuleManager::hasOption(const GameManager& game, const RuleType ruleType, const uint16_t selectedValue) const
{
    const std::vector<GameOption> options = evaluate(game);
    return std::ranges::any_of(
        options,
        [ruleType, selectedValue](const GameOption& option) {
            return option.ruleType == ruleType && (selectedValue == 0 || option.selectedValue == selectedValue);
        }
    );
}

void RuleManager::apply(GameManager& game, const GameOption& option) const
{
    Player* player = game.getCurrentPlayer();
    if (!player)
        return;

    Score& score = player->getScore();
    switch (option.ruleType) {
        case RuleType::Strait:
        case RuleType::Set:
        case RuleType::Single:
            score.increaseRoundScore(option.pointsAwarded);
            break;
        case RuleType::Multiple:
            score.setScoreFromMultiples(option.accumulatedMultipleScore);
            score.increaseRoundScore(option.pointsAwarded);
            game.setValueOfChosenMultiple(option.selectedValue);
            break;
        case RuleType::AddMultiple:
            score.increaseRoundScore(option.pointsAwarded);
            score.setScoreFromMultiples(option.accumulatedMultipleScore);
            break;
    }

    consumeDice(game, option);
}

}
