#ifndef ZILCH_PLAYER_H
#define ZILCH_PLAYER_H

#include "Score.h"
#include "Dice.h"

struct Player {
    Dice dice;
    Score score;
    std::string name = "No Name";
    bool isTurn = false;
};

#endif //ZILCH_PLAYER_H
