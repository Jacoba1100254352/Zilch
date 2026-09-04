// Player.h
/**
Author: Jacob Anderson
Class: ECE-6122-A
Last Date Modified: 11/14/2023

Description:
Defines the Player class, encapsulating a player's dice, score, and name with accessor methods.
*/

#ifndef ZILCH_PLAYER_H
#define ZILCH_PLAYER_H

#include <string>
#include <utility>
#include "Dice.h"
#include "Score.h"

/** Player controller kind selected in the native setup screen. */
enum class PlayerKind {
    Human,
    Computer
};

/** Built-in computer policy level. */
enum class ComputerDifficulty {
    Easy,
    Medium,
    Hard
};

/** Setup-time player record used before live scores and dice exist. */
struct PlayerSetup {
    std::string name;
    PlayerKind kind{PlayerKind::Human};
    ComputerDifficulty difficulty{ComputerDifficulty::Medium};
};

/**
 * @class Player Player.h "Player.h"
 *
 * @brief Encapsulates a player's dice, score, name, and controller metadata.
 */
class Player {
public:
    Player() = delete; // Disable the default constructor as we always need a GameManager instance to create a Player

    explicit Player(std::string playerName) : Player({std::move(playerName), PlayerKind::Human, ComputerDifficulty::Medium}) {}
    explicit Player(PlayerSetup setup)
        : name(std::move(setup.name)), kind(setup.kind), difficulty(setup.difficulty) {}

    // No "sets" are included for these as the names don't change and everything else is pass by reference and defined in the constructor
    [[nodiscard]] Dice& getDice() { return dice; }
    [[nodiscard]] const Dice& getDice() const { return dice; }

    // Accessor methods for Score and Name
    [[nodiscard]] Score& getScore() { return score; }
    [[nodiscard]] const Score& getScore() const { return score; }
    [[nodiscard]] std::string getName() const { return name; }
    [[nodiscard]] PlayerKind getKind() const { return kind; }
    [[nodiscard]] bool isComputer() const { return kind == PlayerKind::Computer; }
    [[nodiscard]] ComputerDifficulty getComputerDifficulty() const { return difficulty; }

private:
    Dice dice;
    Score score;
    std::string name;
    PlayerKind kind;
    ComputerDifficulty difficulty;
};

#endif //ZILCH_PLAYER_H
