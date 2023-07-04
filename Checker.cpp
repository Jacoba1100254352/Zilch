#include "Checker.h"
#include "GameManager.h"
#include "Dice.h"
#include "GameUI.h"
#include <iostream>
#include <cmath>
#include <limits>

/*******************
*   DICE HELPERS   *
*******************/

void Checker::manageDiceUpdateCount(uint8_t numOfDice) {
    Player* const& currentPlayer = game.getCurrentPlayer();

    currentPlayer->getDice().setNumDiceInPlay(numOfDice);

    if ((numOfDice == 0) || (numOfDice == FULL_SET_OF_DICE)) {
        Score& score = currentPlayer->getScore();

        game.setValueOfChosenMultiple(0);
        score.setTurnScore(score.getScoreFromMultiples() + score.getScoreFromSingles());
    }
    if (numOfDice == FULL_SET_OF_DICE)
        game.setTurnContinuationStatus(true);
}

/********************
*   CHECK HELPERS   *
********************/

///   BUST   ///
void Checker::handleFirstRollBust() {
    game.getCurrentPlayer()->getDice().displayDice();
    std::cout << "\nYou have busted on the first roll, try again" << std::endl;
    game.getCurrentPlayer()->getScore().setScoreFromSingles(50);
    game.setTurnContinuationStatus(true, true);
}
void Checker::handleBust() {
    game.getCurrentPlayer()->getDice().displayDice();
    std::cout << "\nYou have busted" << std::endl;
    manageDiceUpdateCount(0);
    game.getCurrentPlayer()->getScore().setTurnScore(0);
    game.setTurnContinuationStatus(false, true);
}

///   MULTIPLE   ///
void Checker::updateAvailableMultiple() {
    for (const auto& die : game.getCurrentPlayer()->getDice().diceSetMap)
        if (die.second >= 3)
            game.setValueOfAvailableMultiple(die.first);
}
void Checker::printPossibleMultipleAddition() {
    if (canAddMultiples() && (game.getCurrentPlayer()->getScore().getScoreFromMultiples() >= 200))
        std::cout << "You can add to your multiple of " << game.getValueOfChosenMultiple() << "!\n";
}

///   OTHER   ///
void Checker::ignoreRemainingInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<int>::max(), '\n');
}
bool Checker::enterEndTurnOption() {
    uint8_t playOrEndTurn;
    std::cin >> playOrEndTurn;

    while (std::cin.fail() || playOrEndTurn < 0 || 2 < playOrEndTurn) {
        clear();
        std::cout << "Type 2 to end turn, 1 to continue selecting or 0 to roll again: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<int>::max(), '\n');
        std::cin >> playOrEndTurn;
    }

    return (playOrEndTurn == 2) && (game.getCurrentPlayer()->getScore().getRunningScore() >= 1000);
}
void Checker::updateGameStatus(uint8_t playOrEndTurn) {
    game.setTurnContinuationStatus(playOrEndTurn != 2);
    game.setSelectionContinuationStatus(playOrEndTurn == 1);
}
void Checker::handleNoOptionsLeft() {
    clear();
    game.getCurrentPlayer()->getScore().displayCurrentScore(game.getCurrentPlayer()->getName());
    game.getCurrentPlayer()->getDice().displayDice();
    std::cout << "\nThere are no options left";
    pauseAndContinue();
}

/************
*   CHECK   *
************/

void Checker::check() {
    Player* const& currentPlayer = game.getCurrentPlayer();
    Dice& dice = currentPlayer->getDice();
    Score& score = currentPlayer->getScore();

    clear();
    game.setSelectedOptionStatus(false);

    // Handle Busts or full-hand dice re-rolls
    if (!isOptionAvailable()) {
        if (dice.getNumDiceInPlay() == FULL_SET_OF_DICE && !score.getTurnScore()) {
            handleFirstRollBust();
        } else if (dice.getNumDiceInPlay() >= 1) {
            handleBust();
        } else {
            std::cout << "\nYou have a full set of dice now" << std::endl;
            manageDiceUpdateCount(FULL_SET_OF_DICE);
            pauseAndContinue();
            currentPlayer->getScore().displayCurrentScore(currentPlayer->getName());
            return;
        }
        pauseAndContinue();
        game.getCurrentPlayer()->getScore().displayCurrentScore(game.getCurrentPlayer()->getName());
    } else checkUserInput();
}
void Checker::checkUserInput() {
    uint8_t playOrEndTurn = 0;

    Player* const& currentPlayer = game.getCurrentPlayer();
    Score& score = currentPlayer->getScore();

    do {
        clear();

        updateAvailableMultiple();

        printPossibleMultipleAddition();

        if (!game.getSelectedOptionStatus())
            printInstructions(game, GameManager::ENTER);
        else if (isOptionAvailable())
            printInstructions(game, GameManager::NEXT);
        else break;

        ignoreRemainingInput();
        readInput(std::cin);

        if (score.getRunningScore() >= 1000) {
            score.displayHighScoreInfo(currentPlayer->getName(), game.findHighestScoringPlayer()->getName());
            if (enterEndTurnOption()) {
                if (playOrEndTurn == 2 && score.getRunningScore() >= 1000)
                    applyPossibleOptions();

                updateGameStatus(playOrEndTurn);
                clear();
            }
        } else if (!isOptionAvailable()) {
            handleNoOptionsLeft();
            break;
        }
    } while (game.getTurnContinuationStatus() && game.getSelectionContinuationStatus() && isOptionAvailable());
}


void Checker::checkStraits() {
    if (!isStrait())
        return;

    game.getCurrentPlayer()->getScore().setScoreFromSingles(1000);
    game.getCurrentPlayer()->getDice().diceSetMap.clear();
    game.setSelectedOptionStatus(true);
}

void Checker::checkSet() {
    if (!isSet())
        return;

    game.getCurrentPlayer()->getScore().setScoreFromSingles(1000);
    game.getCurrentPlayer()->getDice().diceSetMap.clear();
    game.setSelectedOptionStatus(true);
}

void Checker::checkMultiple(uint16_t val) {
    std::map<uint8_t, uint8_t>& diceSetMap = game.getCurrentPlayer()->getDice().diceSetMap;
    if (!isDesiredMultipleAvailable(val) && !game.getValueOfAvailableMultiple())
        return;

    if (isMultiple()) {
        game.setValueOfChosenMultiple(val);
        uint16_t baseScore = (val == 1) ? 1000 : val * 100;
        uint8_t numMultiples = diceSetMap[val] - 3;
        auto score = static_cast<uint16_t>(pow(2, numMultiples) * baseScore);

        if (diceSetMap[val] > FULL_SET_OF_DICE || diceSetMap[val] - 3 < 0)
            throw std::out_of_range("Error: You have " + std::to_string(diceSetMap[val]) + " dice. \nYou cannot have more than 6 dice or less than 3 multiples.");

        if (std::any_of(diceSetMap.begin(), diceSetMap.end(), [val](const auto& die) { return die.first == val; })) {
            game.getCurrentPlayer()->getScore().setScoreFromMultiples(score);
            game.getCurrentPlayer()->getScore().increaseRunningScore(score);
        }
    } else if (canAddMultiples()) {
        if ((game.getCurrentPlayer()->getScore().getScoreFromMultiples() < 200) || (diceSetMap[val] > 3))
            throw std::out_of_range("Error: You must have had a multiple selected and cannot have more than 6 dice or 3 added multiples");

        if (std::any_of(diceSetMap.begin(), diceSetMap.end(), [val](const auto& die) { return die.first == val; })) {
            uint32_t score = 0; // TODO Calculate score, prob define a function for calculating add-on, pull from other project
            game.getCurrentPlayer()->getScore().setScoreFromMultiples(score); // instead of score, was diceSetMap[val]
            game.getCurrentPlayer()->getScore().increaseRunningScore(score);
        }
    } else return;

    diceSetMap[val] = 0;
    game.getCurrentPlayer()->getDice().recountNumDiceInPlay();
    game.setSelectedOptionStatus(true);
}

void Checker::checkSingle(uint8_t val) {
    Player* currentPlayer = game.getCurrentPlayer();
    Score& currentPlayerScore = currentPlayer->getScore();
    std::map<uint8_t, uint8_t> &diceSetMap = currentPlayer->getDice().diceSetMap;
    if (!((val == 1 && isSingle(val)) || (val == 5 && isSingle(5))))
        return;

    if ((game.getValueOfChosenMultiple() == val) || (diceSetMap[val] >= 3)) {
        std::cout << "The option to chose multiples has been automatically applied.\n" << std::endl;
        checkMultiple(val);
    } else if ((diceSetMap[val] < 3) && (diceSetMap[val] >= 1)) {
        currentPlayerScore.setScoreFromSingles((val == 1) ? 100 : 50);
        diceSetMap[val]--;
        manageDiceUpdateCount(currentPlayer->getDice().getNumDiceInPlay() - 1);
        game.setSelectedOptionStatus(true);
    }
}

void Checker::updateAvailableMultipleValues() {
    const std::map<uint8_t, uint8_t> &diceSetMap = game.getCurrentPlayer()->getDice().diceSetMap;
    for (const auto& die : diceSetMap)
        if (die.second >= 3)
            game.setValueOfAvailableMultiple(die.first);
}

void Checker::applyPossibleOptions() {
    while (isOptionAvailable()) {
        if (isStrait())
            checkStraits();
        if (isSet())
            checkSet();
        for (uint8_t i = 1; i <= FULL_SET_OF_DICE; i++)
            if ((isMultiple() && isDesiredMultipleAvailable(i)) || ((game.getCurrentPlayer()->getScore().getScoreFromMultiples() >= 200) && (game.getValueOfChosenMultiple() == i) && canAddMultiples()))
                checkMultiple(i);

        if (isSingle(1))
            checkSingle(1);
        if (isSingle(5))
            checkSingle(5);
    }
}

void Checker::checkLastTurnOpportunity() {
    Player* gameEndingPlayer = game.getCurrentPlayer();

    std::cout << gameEndingPlayer->getName() << " is over " << gameEndingPlayer->getScore().getScoreLimit() << "\nEveryone else has one more chance to win" << std::endl;
    pauseAndContinue();
    std::cout << "\n" << std::endl;

    game.switchToNextPlayer();

    do {
        std::cout << "It is " << game.getCurrentPlayer()->getName() << "'s last turn" << std::endl;
        manageDiceUpdateCount(FULL_SET_OF_DICE);

        while (game.getTurnContinuationStatus())
            game.getCurrentPlayer()->getDice().rollSixDice();

        game.switchToNextPlayer();
    } while (game.getCurrentPlayer() != gameEndingPlayer);
}

void Checker::checkTiedEnding() {
    Player* playerWithHighestScore = game.findHighestScoringPlayer();
    uint32_t highestScore = playerWithHighestScore->getScore().getPermanentScore();
    std::vector<Player*> tie;

    for (Player& player : game.getAllPlayers())
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

bool Checker::isStrait() const {
    return (game.getCurrentPlayer()->getDice().diceSetMap.size() == FULL_SET_OF_DICE);
}

bool Checker::isSet() const {
    return (game.getCurrentPlayer()->getDice().diceSetMap.size() == 3 && !isStrait() && !isMultiple() &&
            !std::any_of(game.getCurrentPlayer()->getDice().diceSetMap.begin(), game.getCurrentPlayer()->getDice().diceSetMap.end(), [](const auto &die) { return die.second != 2; }));
}

bool Checker::isDesiredMultipleAvailable(uint8_t desiredMultiple) const {
    return (std::any_of(game.getCurrentPlayer()->getDice().diceSetMap.begin(), game.getCurrentPlayer()->getDice().diceSetMap.end(), [desiredMultiple](const auto &die) {
        return die.first == desiredMultiple && die.second >= 3;
    }));
}

///   Checks to verify that Multiples exist   ///
bool Checker::isMultiple() const { // or is/areMultiples
    return (std::any_of(game.getCurrentPlayer()->getDice().diceSetMap.begin(), game.getCurrentPlayer()->getDice().diceSetMap.end(), [](const auto& die) { return die.second >= 3; }));
}

bool Checker::canAddMultiples() const {
    return (std::any_of(game.getCurrentPlayer()->getDice().diceSetMap.begin(), game.getCurrentPlayer()->getDice().diceSetMap.end(), [this](const auto &die) { return die.first == game.getValueOfChosenMultiple(); }));
}

bool Checker::isSingle(uint8_t single) const {
    return ((single == 1 || single == 5) && std::any_of(game.getCurrentPlayer()->getDice().diceSetMap.begin(), game.getCurrentPlayer()->getDice().diceSetMap.end(), [single](const auto &die) { return die.first == single; }));
}

bool Checker::isOptionAvailable() const {
    return (isStrait() || isSet() || isMultiple() || canAddMultiples() || isSingle(1) || isSingle(5));
}


/*******************************
*   INPUT HANDLING FUNCTIONS   *
*******************************/

void Checker::readInput(std::istream& input) {
    Player* currentPlayer = game.getCurrentPlayer();
    Score& score = currentPlayer->getScore();

    char ch = '\0';
    uint8_t val = 0;

    // Keep reading input until we get a meaningful character
    do input >> ch;
    while (ch == ' ' || (!strchr("stel123456am0?", ch)));

    switch (ch) {
        case '1': case '2': case '3':
        case '4': case '5': case '6':
            val = ch - '0'; // Convert char digit to number
            if (isStrait() || isSet() || (isMultiple() && isDesiredMultipleAvailable(val))
                || ((score.getScoreFromMultiples() >= 200) && (game.getValueOfChosenMultiple() == val) && (canAddMultiples()))
                || isSingle(val))
                executeSelection(val);
            else displayImpossibleOptionMessage();
            break;
        case 's':
            executeSecondaryCommand(input);
            break;
        case 'a':
            input >> ch;
            input.clear();
            input.ignore(std::numeric_limits<int>::max(), '\n');
            if (ch == 'l')
                applyPossibleOptions();

            // No break here, falls through to case 'm' intentionally
        case 'm':
            input >> val;
            if (canProcessMultiple(val))
                checkMultiple(val);
            else handleFailedMultiple(val);
            break;
        case '0':
            processZeroCommand();
            break;
        case '?':
            displayPossibleOptions();
            break;
        default:
            displayImpossibleOptionMessage();
    }
}

// Execute dice selection based on the value
void Checker::executeSelection(uint8_t val) {
    if (isStrait())
        checkStraits();
    else if (isSet())
        checkSet();
    else if (isMultiple() && isDesiredMultipleAvailable(val))
        checkMultiple(val);
    else if (isSingle(val))
        checkSingle(val);
}

// Handles 's' command
void Checker::executeSecondaryCommand(std::istream& input) {
    char ch;
    input >> ch;
    if ((ch == 't' && isStrait()) || (ch == 'e' && isSet()))
        executeSelection(ch);
    else if ((ch == '1' && isSingle(1)) || (ch == '5' && isSingle(5)))
        checkSingle(ch - '0');
    else displayImpossibleOptionMessage();
}

// Handles failed 'm' command
bool Checker::canProcessMultiple(uint8_t val) {
    return (isMultiple() && isDesiredMultipleAvailable(val)) || ((game.getCurrentPlayer()->getScore().getScoreFromMultiples() >= 200) && (game.getValueOfChosenMultiple() == val) && (canAddMultiples()));
}
void Checker::handleFailedMultiple(uint8_t val) {
    std::cout << "You have selected an impossible option" << std::endl;
    updateAvailableMultipleValues();
    uint8_t chosenMultiple = game.getValueOfChosenMultiple();
    uint8_t availableMultiple = game.getValueOfAvailableMultiple();
    if (chosenMultiple)
        std::cout << "The Value of the multiple is: " << chosenMultiple << std::endl;
    else if (availableMultiple)
        std::cout << "The Value of the multiple is: " << availableMultiple << std::endl;
    else {
        clear();
        std::cout << "There is no available multiple" << std::endl;
    }
}

// Handles '0' command
void Checker::processZeroCommand() {
    Score& score = game.getCurrentPlayer()->getScore();
    if (game.getSelectedOptionStatus())
        game.setSelectionContinuationStatus(false);
    else if ((score.getRunningScore() >= 1000) && isOptionAvailable()) {
        score.increasePermanentScore(score.getScoreFromSingles());
        score.setScoreFromSingles(0);
        std::cout << "Your official score is now: " << score.getPermanentScore() << std::endl;
        manageDiceUpdateCount(FULL_SET_OF_DICE);
        game.switchToNextPlayer();
        std::cout << "\nIt is now " << game.getCurrentPlayer() << "'s turn" << std::endl;
    } else if (!isOptionAvailable()) {
        clear();
    } else {
        clear();
        std::cout << "You cannot end without a score higher than 1000" << std::endl;
        pauseAndContinue();
        score.displayCurrentScore(game.getCurrentPlayer()->getName());
    }
}

// Handles '?' command
void Checker::displayPossibleOptions() {
    std::cout << "Possible options: " << std::endl;
    if (isStrait())
        std::cout << "\tStrait" << std::endl;
    if (isSet())
        std::cout << "\tSets" << std::endl;
    if (isMultiple())
        std::cout << "\tMultiples, value: " << game.getValueOfAvailableMultiple() << std::endl;
    if (canAddMultiples())
        std::cout << "\tAddons, value: " << game.getValueOfChosenMultiple() << std::endl;
    if (isSingle(1) || isSingle(5))
        std::cout << "\tSingles" << std::endl;
}

// Display common impossible option message
void Checker::displayImpossibleOptionMessage() {
    clear();
    std::cout << "You have selected an impossible option" << std::endl;
}

