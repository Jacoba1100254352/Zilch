#ifndef ZILCH_RULES_RULE_CONTEXT_H
#define ZILCH_RULES_RULE_CONTEXT_H

#include "Dice.h"
#include "GameManager.h"
#include "Player.h"
#include "RuleConfig.h"
#include "Score.h"

namespace zilch::rules {

struct RuleContext {
    const GameManager& game;
    const Player& player;
    const Dice& dice;
    const Score& score;
    const RuleConfig& config;
};

}

#endif // ZILCH_RULES_RULE_CONTEXT_H
