#include "Dice.h"
#include <random>

uint8_t Dice::roll() {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<> dist(1, 7);
    return static_cast<uint8_t>(dist(mt));
}

void Dice::rollSixDice(Zilch& game, Checker& checker) {
    clear();
    game.getCurrentPlayer()->dice.diceSetMap.clear();
    game.setOptionSelectedBool(false);
    game.setContinueSelectingBool(true);


    for (uint8_t i = 0; i < game.getNumOfDiceInPlay(); i++)
        game.getCurrentPlayer()->dice.diceSetMap[roll()]++;

    checker.check();
}
