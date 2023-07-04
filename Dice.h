#ifndef ZILCH_DICE_H
#define ZILCH_DICE_H

#include <cstdint>
#include <iostream>
#include <map>

#define FULL_SET_OF_DICE 6

class Dice {
public:
    Dice() : numDiceInPlay(6) {} // Constructor initializes the member variable

    std::map<uint8_t, uint8_t> diceSetMap;

    void rollSixDice();

    [[nodiscard]] uint8_t getNumDiceInPlay() const;
    void setNumDiceInPlay(uint8_t numOfDice);

    void recountNumDiceInPlay();

    void displayDice();

private:
    uint8_t numDiceInPlay;
};

#endif //ZILCH_DICE_H
