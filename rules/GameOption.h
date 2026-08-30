#ifndef ZILCH_RULES_GAME_OPTION_H
#define ZILCH_RULES_GAME_OPTION_H

#include <cstdint>
#include <map>
#include <string>

namespace zilch::rules {

enum class RuleType {
    Straight,
    Strait = Straight,
    ThreePairs,
    Set = ThreePairs,
    Multiple,
    AddMultiple,
    Single
};

struct GameOption {
    RuleType ruleType;
    std::string label;
    uint16_t selectedValue{0};
    uint32_t pointsAwarded{0};
    uint32_t accumulatedMultipleScore{0};
    std::map<uint16_t, uint16_t> consumedDice;
};

}

#endif // ZILCH_RULES_GAME_OPTION_H
