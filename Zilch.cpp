#include "Zilch.h"
#include "Roller.h"
#include "Checker.h"

void Zilch::playGame() {
    initializeGame();
    printGameInfo();

    while (true) {
        std::cout << "Enter the number of players (1-6): ";
        uint8_t numPlayers;
        std::cin >> numPlayers;

        if (numPlayers >= 1 && numPlayers <= 6) {
            for (uint8_t i = 0; i < numPlayers; i++) {
                std::string playerName;
                std::cout << "Enter the name of player " << i + 1 << ": ";
                std::cin >> playerName;
                addPlayer(playerName);
            }
            break;
        } else {
            std::cout << "Invalid number of players. Please try again." << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<int>::max(), '\n');
        }
    }

    while (true) {
        std::cout << "\nEnter the score limit (minimum 1000): ";
        std::cin >> scoreLimit;

        if (scoreLimit >= 1000)
            break;
        else
            std::cout << "Invalid score limit. Please try again." << std::endl;
    }

    std::cout << "\nGame start!\n" << std::endl;
    pauseAndContinue(*this, true);

    while (true) {
        for (const auto& player : players) {
            if (getPermanentScore(player) >= scoreLimit) {
                Checker::lastTurnOpportunity(*this, numOfDiceInPlay);
                Checker::tiedEnding(*this);
                return;
            }

            currentPlayer = player;
            numOfDiceInPlay = 6;
            while (continueTurnBool)
                Roller::rollSixDice(*this);

            pauseAndContinue(*this, true);
        }
    }
}

void Zilch::initializeGame() {
    players.clear();
    currentPlayer.clear();
    numOfDiceInPlay = 6;
    turnScore = 0;
    runningScore = 0;
    scoreFromMultiples = 0;
    scoreFromSingles = 0;
    valOfChosenMultiple = 0;
    valOfAvailableMultiple = 0;
    optionSelectedBool = false;
    continueTurnBool = false;
    continueSelectingBool = false;
}

void Zilch::printGameInfo() {
    std::cout << "Welcome to Zilch!" << std::endl;
    std::cout << "-----------------" << std::endl;
    std::cout << "Rules:" << std::endl;
    std::cout << "- Each player takes turns rolling six dice." << std::endl;
    std::cout << "- The goal is to score as many points as possible." << std::endl;
    std::cout << "- Points can be earned by rolling multiples of the same number, straights, or a set of six dice." << std::endl;
    std::cout << "- Rolling a single 1 awards 100 points, and a single 5 awards 50 points." << std::endl;
    std::cout << "- Rolling three or more of a kind awards points based on the value of the multiple." << std::endl;
    std::cout << "- Rolling a straight (1, 2, 3, 4, 5, 6) awards 1000 points." << std::endl;
    std::cout << "- Rolling a set of six dice awards 1000 points." << std::endl;
    std::cout << "- The first player to reach the score limit wins the game." << std::endl;
    std::cout << "-------------------------------------------------------" << std::endl;
}

void Zilch::addPlayer(const std::string& playerName) {
    players.push_back(playerName);
}

uint32_t Zilch::getPermanentScore(const std::string& playerName) const {
    // Add your implementation here
    return 0;
}

std::string Zilch::getHighestScoringPlayer() const {
    // Add your implementation here
    return "";
}

uint8_t Zilch::getAmountOfPlayers() const {
    return players.size();
}

uint8_t Zilch::getNumOfDiceInPlay() const {
    return numOfDiceInPlay;
}

void Zilch::setNumOfDiceInPlay(int8_t change) {
    // Add your implementation here
}

uint32_t Zilch::getTurnScore() const {
    return turnScore;
}

void Zilch::setTurnScores(uint32_t score) {
    turnScore = score;
}

uint32_t Zilch::getRunningScore() const {
    return runningScore;
}

void Zilch::setRunningScore(uint32_t score) {
    runningScore = score;
}

uint32_t Zilch::getScoreFromMultiples() const {
    return scoreFromMultiples;
}

void Zilch::setScoreFromMultiples(uint32_t score) {
    scoreFromMultiples = score;
}

uint32_t Zilch::getScoreFromSingles() const {
    return scoreFromSingles;
}

void Zilch::setScoreFromSingles(uint32_t score) {
    scoreFromSingles = score;
}

uint32_t Zilch::getScoreLimit() const {
    return scoreLimit;
}

uint8_t Zilch::getValOfChosenMultiple() const {
    return valOfChosenMultiple;
}

void Zilch::setValOfChosenMultiple(uint8_t val) {
    valOfChosenMultiple = val;
}

uint8_t Zilch::getValOfAvailableMultiple() const {
    return valOfAvailableMultiple;
}

void Zilch::setValOfAvailableMultiple(uint8_t val) {
    valOfAvailableMultiple = val;
}

bool Zilch::isOptionAvailable() const {
    return (isStrait() || isSet() || canAddMultiples() || isSingle(1) || isSingle(5));
}

bool Zilch::getOptionSelectedBool() const {
    return optionSelectedBool;
}

void Zilch::setOptionSelectedBool(bool value) {
    optionSelectedBool = value;
}

bool Zilch::getContinueTurnBool() const {
    return continueTurnBool;
}

bool Zilch::getContinueSelectingBool() const {
    return continueSelectingBool;
}

void Zilch::setContinueTurnBool(bool turnBool, bool selectingBool) {
    continueTurnBool = turnBool;
    continueSelectingBool = selectingBool;
}

bool Zilch::isStrait() const {
    // Add your implementation here
    return false;
}

bool Zilch::isSet() const {
    // Add your implementation here
    return false;
}

bool Zilch::isDesiredMultipleAvailable(uint8_t val) const {
    // Add your implementation here
    return false;
}

bool Zilch::canAddMultiples() const {
    // Add your implementation here
    return false;
}

bool Zilch::isSingle(uint8_t val) const {
    // Add your implementation here
    return false;
}

std::istream& operator>>(std::istream& is, Zilch& game) {
    // Add your implementation here
    return is;
}

void pauseAndContinue(const Zilch& game, bool showCurrentScore) {
    // Add your implementation here
}

void showDice(const Zilch& game) {
    // Add your implementation here
}

void clear() {
    std::cout << "\033[2J\033[1;1H";
}

void printInstructions(const Zilch& game, int options) {
    // Add your implementation here
}
