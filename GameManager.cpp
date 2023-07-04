#include "GameManager.h"
#include "Checker.h"
#include "GameUI.h"
#include <numeric>


void GameManager::playGame() {
    Checker gameChecker(*this);

    displayGameInfo();

    // Setup Players
    enterAndAddPlayers();
    enterAndSetScoreLimit();

    std::cout << "\nGame start!\n" << std::endl;

    while (true) {
        for (Player &player: players) {
            pauseAndContinue();
            currentPlayer->getScore().displayCurrentScore(currentPlayer->getName());

            if (player.getScore().getPermanentScore() >= player.getScore().getScoreLimit()) {
                gameChecker.checkLastTurnOpportunity();
                gameChecker.checkTiedEnding();
                return;
            }

            currentPlayer = &player;
            while (turnContinuationStatus) {
                currentPlayer->getDice().rollSixDice();
                gameChecker.check();
            }
        }
    }
}

/*****************************
*   PLAYER SETUP FUNCTIONS   *
*****************************/

///   Player Setup   ///
static uint16_t getNumberOfPlayers() {
    // Variables
    uint16_t numPlayers;

    // Enter and Validate numPlayers
    while (true) {
        std::cout << "Enter the number of players (1-6): ";
        std::cin >> numPlayers;

        // Exit on correct input
        if (!std::cin.fail() && 1 <= numPlayers && numPlayers <= 6)
            break;

        // Reinput
        std::cout << "Invalid number of players. Please try again." << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<int>::max(), '\n');
    }

    return numPlayers;
}
void GameManager::enterAndAddPlayers() {
    uint16_t numPlayers = getNumberOfPlayers();

    players.clear();  // Clears any previous players
    for (uint16_t i = 0; i < numPlayers; i++) {
        std::string playerName;
        std::cout << "Enter the name of player " << i + 1 << ": ";
        std::cin >> playerName;
        addPlayer(playerName);
    }

    // Initialize currentPlayer to be the first player
    currentPlayer = &players.at(0);
}
void GameManager::addPlayer(const std::string& playerName) {
    players.emplace_back(playerName); // Pass this GameManager instance to the player
}
std::vector<Player> GameManager::getAllPlayers() const {
    return players;
}

///   Player Sequencing   ///
Player* GameManager::getCurrentPlayer() const {
    return currentPlayer;
}
void GameManager::switchToNextPlayer() {
    static size_t currentPlayerIndex = 0;
    currentPlayerIndex = ++currentPlayerIndex % players.size();
    currentPlayer = &players[currentPlayerIndex];
}

///   Score Limit and Highest Score   ///
void GameManager::enterAndSetScoreLimit() {
    // Variables
    const uint32_t MIN_SCORE_LIMIT = 1000;
    uint32_t limit;

    // Enter and Validate scoreLimit
    while (true) {
        std::cout << "Enter the score limit (minimum " << MIN_SCORE_LIMIT << "): ";
        std::cin >> limit;

        // Exit on correct input
        if (!std::cin.fail() && limit >= MIN_SCORE_LIMIT)
            break;

        // Re-input
        std::cout << "Invalid score limit. Please try again." << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<int>::max(), '\n');
    }

    // Assign scoreLimit per player
    for (Player& player : players)
        player.getScore().setScoreLimit(limit);
}
Player* GameManager::findHighestScoringPlayer() {
    Player* highestScoringPlayer = currentPlayer;
    for (Player& player: players)
        if (player.getScore().getPermanentScore() > highestScoringPlayer->getScore().getPermanentScore())
            highestScoringPlayer = &player;
    return highestScoringPlayer;
}

/****************************
*   GET AND SET FUNCTIONS   *
****************************/

///   Chosen Multiple   ///
uint8_t GameManager::getValueOfChosenMultiple() const {
    return valueOfChosenMultiple;
}
void GameManager::setValueOfChosenMultiple(uint8_t chosenMultipleValue) {
    valueOfChosenMultiple = chosenMultipleValue;
}

///   Available Multiple   ///
uint8_t GameManager::getValueOfAvailableMultiple() const {
    return valueOfAvailableMultiple;
}
void GameManager::setValueOfAvailableMultiple(uint8_t availableMultipleValue) {
    valueOfAvailableMultiple = availableMultipleValue;
}

///   Option Status   ///
bool GameManager::getSelectedOptionStatus() const {
    return selectedOptionStatus;
}
void GameManager::setSelectedOptionStatus(bool value) {
    selectedOptionStatus = value;
}

///   Turn Continuation Status   ///
bool GameManager::getTurnContinuationStatus() const {
    return turnContinuationStatus;
}
void GameManager::setTurnContinuationStatus(bool continueTurn, bool isBust) {
    // If continueTurn is true or isBust is true, we continue with the current status
    if (isBust) {
        turnContinuationStatus = continueTurn;
        return;
    }

    // We opted to stop: Now, we know that both continueTurn and isBust are false.
    Score& score = currentPlayer->getScore();

    // Greater than or equal to 1000: log and end turn
    if (score.getRunningScore() >= 1000) {
        score.setTurnScore(score.getScoreFromSingles() + score.getScoreFromMultiples());
        score.increasePermanentScore(score.getTurnScore());
        std::cout << std::endl << currentPlayer->getName() << "'s permanent score has been logged and is now: " << score.getPermanentScore() << std::endl << std::endl;
        currentPlayer->getDice().setNumDiceInPlay(0);
        // else if: No option selected and not above 1000: select an option
    } else if (!selectedOptionStatus) {
        std::cout << "You are not allowed to end without a permanent or running score higher than 1000" << std::endl;
        continueTurn = true;
        //zilch::pauseAndContinue(game, 1 );
        //printInstructions(game, REENTER);
    }

    // Set the turn continuation status
    turnContinuationStatus = continueTurn;
}

///   Selection Status   ///
bool GameManager::getSelectionContinuationStatus() const {
    return selectionContinuationStatus;
}
void GameManager::setSelectionContinuationStatus(bool continueSelecting) {
    if (!continueSelecting && !selectedOptionStatus) {
        std::cout << "You must select at least one option" << std::endl;
        continueSelecting = true;
    }
    selectionContinuationStatus = continueSelecting;
}
