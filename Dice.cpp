#include "Dice.h"
#include <random>

static void removeZeros(std::map<uint8_t, uint8_t>& diceSetMap) {
    for (auto it = diceSetMap.begin(); it != diceSetMap.end(); )
        if (it->second == 0)
            it = diceSetMap.erase(it);
        else ++it;
}

static uint8_t roll() {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<> dist(1, 7);
    return static_cast<uint8_t>(dist(mt));
}
void Dice::rollSixDice() {
    diceSetMap.clear();

    for (uint8_t i = 0; i < getNumDiceInPlay(); i++)
        diceSetMap[roll()]++;
}

[[nodiscard]] uint8_t Dice::getNumDiceInPlay() const {
    return numDiceInPlay;
}

void Dice::setNumDiceInPlay(uint8_t numOfDice) {
    removeZeros(diceSetMap);
    if (numOfDice > 6)
        numDiceInPlay = 6;
    else numDiceInPlay = numOfDice;
}

void Dice::recountNumDiceInPlay() {
    numDiceInPlay = 0;
    for (const auto die : diceSetMap)
        numDiceInPlay += die.second;
}

void Dice::displayDice() {
    printf("\nYou have %d dice left\n", numDiceInPlay);

    ///   Build and Print Dice List String   ///
    std::string diceList;
    for (const auto &die: diceSetMap)
        for (uint8_t i = 0; i < die.second; i++)
            diceList += std::to_string(die.first) + ", ";

    // Remove the trailing comma and space
    if (!diceList.empty())
        diceList.erase(diceList.length() - 2);

    std::cout << diceList << std::endl;
}

