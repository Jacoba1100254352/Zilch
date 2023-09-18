#include "Dice.h"
#include <random>

static void removeZeros(std::map<uint16_t, uint16_t>& diceSetMap) {
    for (auto it = diceSetMap.begin(); it != diceSetMap.end(); )
        if (it->second == 0)
            it = diceSetMap.erase(it);
        else ++it;
}

static uint16_t roll() {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<> dist(1, 7);
    return static_cast<uint16_t>(dist(mt));
}
void Dice::rollDice() {
    diceSetMap.clear();
    multipleExists = false;

    for (uint16_t i = 0; i < getNumDiceInPlay(); i++) {
        uint16_t dieValue = roll();
        if (++diceSetMap[dieValue] >= 3)
            multipleExists = true;
    }
}

[[nodiscard]] uint16_t Dice::getNumDiceInPlay() const {
    return numDiceInPlay;
}
void Dice::setNumDiceInPlay(uint16_t numOfDice) {
    removeZeros(diceSetMap);

    if (numOfDice > 6)
        numDiceInPlay = 6;
    else numDiceInPlay = numOfDice;
}

void Dice::calculateMultipleAvailability() {
    multipleExists = false;

    for (const auto die : diceSetMap)
        if (die.second >= 3)
            multipleExists = true;
}
bool Dice::isMultipleAvailable() const {
    return multipleExists;
}

void Dice::eliminateDice(uint16_t dieValue) {
    diceSetMap[dieValue] = 0;
    removeZeros(diceSetMap);
}

[[maybe_unused]] void Dice::recountNumDiceInPlay() {
    removeZeros(diceSetMap);

    numDiceInPlay = 0;
    for (const auto die : diceSetMap)
        numDiceInPlay += die.second;
}

void Dice::displayDice() {
    printf("\nYou have %d dice left\n", numDiceInPlay);

    ///   Build and Print Dice List String   ///
    std::string diceList;
    for (const auto &die: diceSetMap)
        for (uint16_t i = 0; i < die.second; i++)
            diceList += std::to_string(die.first) + ", ";

    // Remove the trailing comma and space
    if (!diceList.empty())
        diceList.erase(diceList.length() - 2);

    std::cout << diceList << std::endl;
}

