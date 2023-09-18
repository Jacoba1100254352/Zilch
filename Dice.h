#ifndef ZILCH_DICE_H
#define ZILCH_DICE_H

#include <cstdint>
#include <iostream>
#include <map>

#define FULL_SET_OF_DICE 6

class Dice {
public:
    Dice() : numDiceInPlay(FULL_SET_OF_DICE) {} // Constructor initializes the member variable

    std::map<uint16_t, uint16_t> diceSetMap;

    void rollDice();

    [[nodiscard]] uint16_t getNumDiceInPlay() const;
    void setNumDiceInPlay(uint16_t numOfDice);

    [[maybe_unused]] void recountNumDiceInPlay();
    void eliminateDice(uint16_t dieValue);

    void calculateMultipleAvailability();
    bool isMultipleAvailable() const;

    void displayDice();

private:
    uint16_t numDiceInPlay;
    bool multipleExists;
};

#endif //ZILCH_DICE_H
