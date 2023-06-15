#ifndef ZILCH_CHECKER_H
#define ZILCH_CHECKER_H


#include "oldZilch.h"

class Checker {
public:
    static void check(zilch& game);
    static void checkingUserInput(zilch& game);
    static void straits(zilch& game);
    static void set(zilch& game);
    static void multiple(zilch& game, uint8_t val);
    static void single(zilch& game, uint8_t val);
    static void updateValOfAvailableMultiples(zilch& game);
    static void applyAllPossibleOptions(zilch& game);
    static void lastTurnOpportunity(zilch& game, int8_t FULL_SET_OF_DICE);
    static void tiedEnding(zilch& game);
};


#endif //ZILCH_CHECKER_H
