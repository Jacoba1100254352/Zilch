#ifndef ZILCH_DIE_H
#define ZILCH_DIE_H

#include <cstdint>
#include "oldZilch.h"

class Die {
public:
    static uint8_t roll();
    static void rollSixDice(zilch& game);
};

#endif //ZILCH_DIE_H
