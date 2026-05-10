#ifndef ZILCH_RULES_RULE_MANAGER_H
#define ZILCH_RULES_RULE_MANAGER_H

#include <cstdint>
#include <memory>
#include <vector>

#include "RuleConfig.h"
#include "rules/GameOption.h"
#include "rules/IRule.h"

class GameManager;

namespace zilch::rules {

class RuleManager {
public:
    explicit RuleManager(const RuleConfig& config);

    [[nodiscard]] std::vector<GameOption> evaluate(const GameManager& game) const;
    [[nodiscard]] bool hasOption(const GameManager& game, RuleType ruleType, uint16_t selectedValue = 0) const;
    void apply(GameManager& game, const GameOption& option) const;

private:
    const RuleConfig& config;
    std::vector<std::unique_ptr<IRule>> rules;
};

}

#endif // ZILCH_RULES_RULE_MANAGER_H
