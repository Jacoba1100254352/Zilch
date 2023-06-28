#ifndef ZILCH_DICE_H
#define ZILCH_DICE_H

#include <cstdint>
#include "Zilch.h"
#include "Checker.h"

struct Dice {
    std::map<uint8_t, uint8_t> diceSetMap;
    //static uint8_t numOfDiceInPlay;

    static uint8_t roll();
    static void rollSixDice(Zilch& game, Checker& checker);
};

#endif //ZILCH_DICE_H
