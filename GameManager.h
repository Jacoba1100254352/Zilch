// GameManager.h

#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include "ZilchStates.h"

// Forward declarations
class Player;

// A small Score struct for illustration (simpler than your original).
struct Score {
    uint32_t limit    = 2000;
    uint32_t perm     = 0;  // permanent (overall) score
    uint32_t round    = 0;  // score during the turn
    uint32_t multiple = 0;  // track multiple scoring in progress
};

// A small Dice class for illustration.
class Dice {
public:
    Dice() : numDiceInPlay(6) {}
    void rollDice();
    void displayDice() const;
    [[nodiscard]] uint16_t getNumDiceInPlay() const { return numDiceInPlay; }
    void setNumDiceInPlay(const uint16_t nd) { numDiceInPlay = nd; }
    void clear() { diceSetMap.clear(); }
    // etc.

    std::map<uint16_t, uint16_t> diceSetMap;

private:
    uint16_t numDiceInPlay;
};

// For demonstration, a trivial Player class
class Player {
public:
    explicit Player(std::string  n) : name(std::move(n)) {}
    Score score;
    Dice dice;
    std::string name;
};

class GameManager {
public:
    GameManager();

    void run();  // Main function that runs the game using the state machine.

private:
    ZilchGameState currentState;

    // Basic game data
    std::vector<Player> players;
    int currentPlayerIndex;

    // === Setup and general flow methods
    void doSetupState();
    void doBeginTurn();
    void doRollDice();
    void doCheckOptions();
    void doSelectOption();
    void doApplyScore();
    void doEndTurn();
    void doCheckGameEnd();
    void doLastTurn();

    // Utility
    [[nodiscard]] bool isGameOver() const;
    Player& getCurrentPlayer();
    void switchToNextPlayer();
};

#endif // GAMEMANAGER_H
