#ifndef ZILCH_PLAYER_H
#define ZILCH_PLAYER_H

#include <utility>
#include "Dice.h"
#include "Score.h"

class Player {
public:
    Player() = delete; // Disable the default constructor as we always need a GameManager instance to create a Player

    explicit Player(std::string playerName) : dice(), score(), name(std::move(playerName)), isTurn(false) {} // Pass GameManager instance to Dice

    // No "sets" are included for these as the names don't change and everything else is pass by reference and defined in the constructor
    [[nodiscard]] Dice& getDice() { return dice; }
    [[nodiscard]] Score& getScore() { return score; }
    [[nodiscard]] std::string getName() const { return name; }

    [[nodiscard]] bool getIsTurn() const { return isTurn; }
    void setIsTurn(bool turnStatus) { isTurn = turnStatus; }

private:
    Dice dice;
    Score score;
    std::string name;
    bool isTurn;
};

#endif //ZILCH_PLAYER_H
