#ifndef ZILCH_GAME_MANAGER_H
#define ZILCH_GAME_MANAGER_H

#include <map>
#include <vector>
#include <stdexcept>
#include "Player.h"

class GameManager {
public:
    GameManager() : players({}), currentPlayer(nullptr), valueOfChosenMultiple(0), valueOfAvailableMultiple(0), selectedOptionStatus(false), turnContinuationStatus(true), selectionContinuationStatus(false) {}

    /************************
    *   ENUM FOR PRINTING   *
    ************************/
    enum printOptions {
        ENTER, NEXT, REENTER
    };

    void playGame();

    [[nodiscard]] Player* findHighestScoringPlayer();
    void switchToNextPlayer();
    [[nodiscard]] std::vector<Player> getAllPlayers() const;

    /**************************
    *   GETTERS AND SETTERS   *
    **************************/

    // SelectedOptionStatus
    [[nodiscard]] bool getSelectedOptionStatus() const;
    void setSelectedOptionStatus(bool optionSelected);

    // TurnContinuationStatus
    [[nodiscard]] bool getTurnContinuationStatus() const;
    void setTurnContinuationStatus(bool continueTurn, bool isBust = false);

    // SelectionContinuationStatus
    [[nodiscard]] bool getSelectionContinuationStatus() const;
    void setSelectionContinuationStatus(bool continueSelecting);

    // CurrentPlayer
    [[nodiscard]] Player* getCurrentPlayer() const;

    // ValueOfChosenMultiple
    [[nodiscard]] uint8_t getValueOfChosenMultiple() const;
    void setValueOfChosenMultiple(uint8_t valueOfChosenMultiple);

    // ValueOfAvailableMultiple
    [[nodiscard]] uint8_t getValueOfAvailableMultiple() const;
    void setValueOfAvailableMultiple(uint8_t valueOfAvailableMultiple);

private:
    std::vector<Player> players;
    Player* currentPlayer;
    uint8_t valueOfChosenMultiple;
    uint8_t valueOfAvailableMultiple;
    bool selectedOptionStatus;
    bool turnContinuationStatus;
    bool selectionContinuationStatus;

    void enterAndAddPlayers();
    void addPlayer(const std::string& playerName);

    void enterAndSetScoreLimit();
};


#endif //ZILCH_GAME_MANAGER_H
