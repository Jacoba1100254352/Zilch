#ifndef ZILCH_ZILCH_H
#define ZILCH_ZILCH_H

#include <iostream>
#include <map>
#include <vector>
#include <stdexcept>
#include "Player.h"

#if defined(_WIN32) || defined(_WIN64) || (defined(__CYGWIN__) && !defined(_WIN32))
#define PLATFORM_NAME "windows" // Windows
#elif defined(__ANDROID__)
#define PLATFORM_NAME "android" // Android (implies Linux, so it must come first)
#elif defined(__linux__)
#define PLATFORM_NAME "linux" // Debian, Ubuntu, Gentoo, Fedora, openSUSE, RedHat, Centos and other
#elif defined(__unix__) || (!defined(__APPLE__) && defined(__MACH__))
#include <sys/param.h>
#if defined(BSD)
#define PLATFORM_NAME "bsd" // FreeBSD, NetBSD, OpenBSD, DragonFly BSD
#endif
#elif defined(__hpux)
#define PLATFORM_NAME "hp-ux" // HP-UX
#elif defined(_AIX)
#define PLATFORM_NAME "aix" // IBM AIX
#elif defined(__APPLE__) && defined(__MACH__) // Apple OSX and iOS (Darwin)
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR == 1 || TARGET_OS_IPHONE == 1
#define PLATFORM_NAME "ios" // Apple iOS
#elif TARGET_OS_MAC == 1
#define PLATFORM_NAME "osx" // Apple OSX
#else
#define PLATFORM_NAME "macos" // Apple OS
#endif
#elif defined(__sun) && defined(__SVR4)
#define PLATFORM_NAME "solaris" // Oracle Solaris, Open Indiana
#else
#define PLATFORM_NAME NULL
#endif

class Zilch {
public:
    /************************
    *   ENUM FOR PRINTING   *
    ************************/
    enum printOptions {
        ENTER, NEXT, REENTER
    };

    void playGame();

    void initializeGame();
    void printGameInfo();
    void addPlayer(const Player& playerName);
    [[nodiscard]] Player* getHighestScoringPlayer();
    [[nodiscard]] size_t getAmountOfPlayers() const;
    [[nodiscard]] uint8_t getNumOfDiceInPlay() const;
    void setNumOfDiceInPlay(int8_t numOfDice = -20);
    [[nodiscard]] uint32_t getTurnScore() const;
    void setTurnScores(uint32_t score);
    [[nodiscard]] uint32_t getRunningScore() const;
    void setRunningScore(uint32_t score);
    [[nodiscard]] uint32_t getScoreFromMultiples() const;
    void setScoreFromMultiples(uint32_t score);
    [[nodiscard]] uint32_t getScoreFromSingles() const;
    void setScoreFromSingles(uint32_t score);
    [[nodiscard]] uint32_t getScoreLimit() const;
    [[nodiscard]] uint8_t getValOfChosenMultiple() const;
    void setValOfChosenMultiple(uint8_t val);
    [[nodiscard]] uint8_t getValOfAvailableMultiple() const;
    void setValOfAvailableMultiple(uint8_t val);
    [[nodiscard]] bool isOptionAvailable() const;
    [[nodiscard]] bool getOptionSelectedBool() const;
    void setOptionSelectedBool(bool value);
    [[nodiscard]] bool getContinueTurnBool() const;
    [[nodiscard]] bool getContinueSelectingBool() const;
    void setContinueTurnBool(bool turnBool, bool selectingBool = false);
    void setContinueSelectingBool(bool val) {
        if (!val && !getOptionSelectedBool()) {
            std::cout << "You must select at least one option" << std::endl;
            val = true;
        }
        continueSelectingBool = val;
    }

    ///   Global Variable to keep track of current player   ///
    Player* getCurrentPlayer() {
        return currentPlayer;
    }
    void incCurrentPlayer() {
        for (int i = 0; i < players.size(); i++)
            if (players.at(i).isTurn) {
                players.at(i).isTurn = false;

                size_t nextIndex = (i+1) % players.size();
                players.at(nextIndex).isTurn = true;
                *currentPlayer = players.at(nextIndex);

                break;
            }
        setContinueTurnBool(true);
    }

    ///   Global Variable containing the players   ///
    std::vector<Player> getPlayers() {
        return players;
    }

    friend std::istream& operator>>(std::istream& is, Zilch& game);

private:
    std::vector<Player> players;
    Player* currentPlayer;
    uint8_t valOfChosenMultiple;
    uint8_t valOfAvailableMultiple;
    bool optionSelectedBool;
    bool continueTurnBool;
    bool continueSelectingBool;
};

void pauseAndContinue(const Zilch& game, bool showCurrentScore = false);
void showDice(const Zilch& game);
void clear();
void printInstructions(const Zilch& game, int options);



#endif //ZILCH_ZILCH_H
