// GameManager.cpp

#include "GameManager.h"
#include "Checker.h"
#include "GameUI.h"


void GameManager::initializeRollCycle()
{
    /***
     * Initialize each segment of 6-dice re-rolls
     */

    turnContinuationStatus = true;
    selectedOptionStatus = false; // No option has yet be selected
    selectionContinuationStatus = true; // Can continue/start selecting // TODO: May need to be set to false

    valueOfChosenMultiple = 0;

    // Initialize Score
    currentPlayer->getScore().setScoreFromMultiples(0);
}

void GameManager::playGame()
{
    displayGameInfo();
    pauseAndContinue();

    clear();

    // Setup Players
    enterAndAddPlayers();
    enterAndSetScoreLimit();

    while (true) {
        for (Player& player : players) {
            // Check and manage game-end condition
            if (player.getScore().getPermanentScore() >= player.getScore().getScoreLimit()) {
                manageLastTurnOpportunity();
                manageTiedEnding();
                return;
            }

            // Play Turn
            currentPlayer = &player;
            manageDiceCount(FULL_SET_OF_DICE);
            while (turnContinuationStatus) {
                currentPlayer->getDice().rollDice();
                Checker(*this).check();
            }
        }
    }
}

void GameManager::manageLastTurnOpportunity()
{
    Player* gameEndingPlayer = currentPlayer;

    std::cout << gameEndingPlayer->getName() << " is over " << gameEndingPlayer->getScore().getScoreLimit() << std::endl
        << "Everyone else has one more chance to win" << std::endl;
    pauseAndContinue();
    std::cout << "\n" << std::endl;

    switchToNextPlayer();

    do {
        std::cout << "It is " << currentPlayer->getName() << "'s last turn" << std::endl;
        manageDiceCount(FULL_SET_OF_DICE);

        while (turnContinuationStatus) {
            gameEndingPlayer->getScore().displayHighScoreInfo(currentPlayer->getName(), gameEndingPlayer->getName());
            currentPlayer->getDice().rollDice();
            Checker(*this).check();
        }

        switchToNextPlayer();
    } while (currentPlayer != gameEndingPlayer);
}

void GameManager::manageTiedEnding()
{
    Player* playerWithHighestScore = findHighestScoringPlayer();
    const uint32_t highestScore = playerWithHighestScore->getScore().getPermanentScore();
    std::vector<Player*> tie;

    for (Player& player : players)
        if (player.getScore().getPermanentScore() == highestScore)
            tie.emplace_back(&player);

    if (tie.size() > 1) {
        for (size_t i = 0; i < tie.size(); ++i) {
            std::cout << tie[i]->getName();
            if (i != tie.size() - 1)
                std::cout << ((i == tie.size() - 2) ? " and " : ", ");
        }
        std::cout << " have tied with " << highestScore << " Points!" << std::endl;
    } else std::cout << playerWithHighestScore->getName() << " won with " << highestScore << " Points!" << std::endl;
}

/*******************
*   DICE HELPERS   *
*******************/

void GameManager::manageDiceCount(uint16_t numOfDice)
{
    if (numOfDice == 0)
        numOfDice = FULL_SET_OF_DICE;

    currentPlayer->getDice().setNumDiceInPlay(numOfDice);

    if (numOfDice == FULL_SET_OF_DICE)
        initializeRollCycle();
}

/*****************************
*   PLAYER SETUP FUNCTIONS   *
*****************************/

///   Player Setup   ///
static uint16_t getNumberOfPlayers()
{
    // Variables
    uint16_t numPlayers;

    // Enter and Validate numPlayers
    while (true) {
        std::cout << "Enter the number of players (1-6): ";
        std::cin >> numPlayers;

        // Exit on correct input
        if (!std::cin.fail() && 1 <= numPlayers && numPlayers <= 6)
            break;

        // Re-input
        std::cout << "Invalid number of players. Please try again." << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<int>::max(), '\n');
    }

    return numPlayers;
}

void GameManager::enterAndAddPlayers()
{
    const uint16_t numPlayers = getNumberOfPlayers();

    players.clear(); // Clears any previous players
    for (uint16_t i = 0; i < numPlayers; i++) {
        std::string playerName;
        std::cout << "Enter the name of player " << i + 1 << ": ";
        std::cin >> playerName;
        addPlayer(playerName);
    }

    // Initialize currentPlayer to be the first player
    currentPlayer = &players.at(0);
}

void GameManager::addPlayer(const std::string& playerName)
{
    players.emplace_back(playerName); // Pass this GameManager instance to the player
}

///   Player Sequencing   ///
Player* GameManager::getCurrentPlayer() const
{
    return currentPlayer;
}

void GameManager::switchToNextPlayer()
{
    static size_t currentPlayerIndex = 0;
    currentPlayerIndex = ++currentPlayerIndex % players.size();
    currentPlayer = &players[currentPlayerIndex];
}

///   Score Limit and Highest Score   ///
void GameManager::enterAndSetScoreLimit()
{
    // Variables
    uint32_t limit;

    // Enter and Validate scoreLimit
    while (true) {
        constexpr uint32_t MIN_SCORE_LIMIT = 1000;
        std::cout << "\nEnter the score limit (minimum " << MIN_SCORE_LIMIT << "): ";
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

Player* GameManager::findHighestScoringPlayer()
{
    Player* highestScoringPlayer = currentPlayer;
    for (Player& player : players)
        if (player.getScore().getPermanentScore() > highestScoringPlayer->getScore().getPermanentScore())
            highestScoringPlayer = &player;
    return highestScoringPlayer;
}

/****************************
*   GET AND SET FUNCTIONS   *
****************************/

///   Chosen Multiple   ///
uint16_t GameManager::getValueOfChosenMultiple() const
{
    return valueOfChosenMultiple;
}

void GameManager::setValueOfChosenMultiple(const uint16_t chosenMultipleValue)
{
    valueOfChosenMultiple = chosenMultipleValue;
}

///   Option Status   ///
bool GameManager::getSelectedOptionStatus() const
{
    return selectedOptionStatus;
}

void GameManager::setSelectedOptionStatus(const bool isOptionSelected)
{
    selectedOptionStatus = isOptionSelected;
}

///   Turn Continuation Status   ///
bool GameManager::getTurnContinuationStatus() const
{
    return turnContinuationStatus;
}

void GameManager::setTurnContinuationStatus(bool continueTurn, const bool isBust)
{
    // If continueTurn is true or isBust is true, we continue with the current status
    if (isBust) {
        turnContinuationStatus = continueTurn;
        return;
    }

    // We opted to stop: Now, we know that both continueTurn and isBust are false.

    // Greater than or equal to 1000: log and end turn
    if (Score& score = currentPlayer->getScore(); score.getRoundScore() >= 1000) {
        score.increasePermanentScore(score.getRoundScore());
        score.setRoundScore(0); // Reset Round Score
        std::cout << std::endl << currentPlayer->getName() << "'s permanent score has been logged and is now: " << score.getPermanentScore() << std::endl << std::endl;
        currentPlayer->getDice().setNumDiceInPlay(0);
        // else if: No option selected and not above 1000: select an option
    } else if (!selectedOptionStatus) {
        std::cout << "You are not allowed to end without a permanent or running score higher than 1000" << std::endl;
        continueTurn = true;
        //pauseAndContinue();
        //printInstructions(game, REENTER);
    }

    // Set the turn continuation status
    turnContinuationStatus = continueTurn;
}

///   Selection Status   ///
bool GameManager::getSelectionContinuationStatus() const
{
    return selectionContinuationStatus;
}

void GameManager::setSelectionContinuationStatus(bool continueSelecting)
{
    if (!continueSelecting && !selectedOptionStatus) {
        std::cout << "You must select at least one option" << std::endl;
        continueSelecting = true;
    }
    selectionContinuationStatus = continueSelecting;
}
