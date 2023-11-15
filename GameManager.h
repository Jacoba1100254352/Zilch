#ifndef ZILCH_GAME_MANAGER_H
#define ZILCH_GAME_MANAGER_H

#include <map>
#include <vector>
#include "Player.h"

class GameManager {
public:
    GameManager() : players({}), currentPlayer(nullptr), valueOfChosenMultiple(0), selectedOptionStatus(false), turnContinuationStatus(true), selectionContinuationStatus(true) {}

    /************************
    *   ENUM FOR PRINTING   *
    ************************/
    enum printOptions {
        ENTER, NEXT, REENTER
    };

    void initializeRollCycle();
    void playGame();

    void manageLastTurnOpportunity();
    void manageTiedEnding();

    void manageDiceCount(uint16_t numOfDice);

    [[nodiscard]] Player* findHighestScoringPlayer();
    void switchToNextPlayer();

    /**************************
    *   GETTERS AND SETTERS   *
    **************************/

    // SelectedOptionStatus
    [[nodiscard]] bool getSelectedOptionStatus() const;
    void setSelectedOptionStatus(bool isOptionSelected);

    // TurnContinuationStatus
    [[nodiscard]] bool getTurnContinuationStatus() const;
    void setTurnContinuationStatus(bool continueTurn, bool isBust = false);

    // SelectionContinuationStatus
    [[nodiscard]] bool getSelectionContinuationStatus() const;
    void setSelectionContinuationStatus(bool continueSelecting);

    // CurrentPlayer
    [[nodiscard]] Player* getCurrentPlayer() const;

    // ValueOfChosenMultiple
    [[nodiscard]] uint16_t getValueOfChosenMultiple() const;
    void setValueOfChosenMultiple(uint16_t chosenMultipleValue);

private:
    std::vector<Player> players;
    Player* currentPlayer;
    uint16_t valueOfChosenMultiple;
    bool selectedOptionStatus;
    bool turnContinuationStatus;
    bool selectionContinuationStatus;

    void enterAndAddPlayers();
    void addPlayer(const std::string& playerName);

    void enterAndSetScoreLimit();
};


#endif //ZILCH_GAME_MANAGER_H
