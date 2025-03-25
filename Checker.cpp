// Checker.cpp

#include "Checker.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <ranges>
#include "Dice.h"
#include "GameManager.h"
#include "GameUI.h"

/********************
*   CHECK HELPERS   *
********************/

///   BUST   ///
void Checker::handleFirstRollBust() const
{
    // bust UI
    game.getCurrentPlayer()->getDice().displayDice();
    std::cout << "\nYou have busted on the first roll, try again" << std::endl;
    pauseAndContinue();

    // Turn and Score updates
    game.getCurrentPlayer()->getScore().increaseRoundScore(50);
    game.setTurnContinuationStatus(true, true);
}

void Checker::handleBust() const
{
    // bust UI
    game.getCurrentPlayer()->getDice().displayDice();
    std::cout << "\nYou have busted" << std::endl;
    pauseAndContinue();

    // Turn and Score updates
    game.getCurrentPlayer()->getScore().setRoundScore(0);
    game.setTurnContinuationStatus(false, true);
}

///   MULTIPLE   ///
void Checker::printPossibleMultipleAddition() const
{
    if (canAddMultiples() && (game.getCurrentPlayer()->getScore().getScoreFromMultiples() >= 200))
        std::cout << "You can add to your multiple of " << game.getValueOfChosenMultiple() << "!\n";
}

///   OTHER   ///
void Checker::ignoreRemainingInput()
{
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<int>::max(), '\n');
}

bool Checker::enterEndTurnOption(uint16_t &playOrEndTurn) const
{

    // Enter Decision
    std::cout << "Type 2 to end turn, 1 to continue selecting or 0 to roll again: ";
    std::cin >> playOrEndTurn;

    // Handle incorrect input
    while (std::cin.fail() || playOrEndTurn < 0 || 2 < playOrEndTurn) {
        clear();
        std::cout << "Type 2 to end turn, 1 to continue selecting or 0 to roll again: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<int>::max(), '\n');
        std::cin >> playOrEndTurn;
    }

    return (playOrEndTurn == 2) && (game.getCurrentPlayer()->getScore().getRoundScore() >= 1000);
}

void Checker::updateGameStatus(const uint16_t playOrEndTurn) const
{
    game.setTurnContinuationStatus(playOrEndTurn != 2);
    game.setSelectionContinuationStatus(playOrEndTurn == 1);
}

void Checker::handleNoOptionsLeft() const
{
    clear();
    game.getCurrentPlayer()->getScore().displayCurrentScore(game.getCurrentPlayer()->getName());
    // TODO: May be unnecessary
    game.getCurrentPlayer()->getDice().displayDice();
    std::cout << "\nThere are no options left";
    pauseAndContinue();
}

/************
*   CHECK   *
************/

void Checker::check() const
{
    Player* const& currentPlayer = game.getCurrentPlayer();
    const Dice& dice = currentPlayer->getDice();
    const Score& score = currentPlayer->getScore();

    //    clear();
    game.setSelectedOptionStatus(false);

    // Handle Busts or full-hand dice re-rolls
    if (!isOptionAvailable()) {
        if (dice.getNumDiceInPlay() == FULL_SET_OF_DICE && !score.getRoundScore()) {
            handleFirstRollBust();
        } else if (dice.getNumDiceInPlay() >= 1) {
            handleBust();
        } else {
            std::cout << "\nYou have a full set of dice now" << std::endl;
            pauseAndContinue();
            game.manageDiceCount(FULL_SET_OF_DICE);
        }
    } else checkUserInput();
}

void Checker::checkUserInput() const
{
    Player* const& currentPlayer = game.getCurrentPlayer();
    const Score& score = currentPlayer->getScore();

    do {
        clear(); // Clear screen.

        printPossibleMultipleAddition(); // Display options for multiple addition.

        // Print instructions based on game status.
        if (!game.getSelectedOptionStatus())
            printInstructions(game, GameManager::ENTER);
        else if (isOptionAvailable())
            printInstructions(game, GameManager::NEXT);
        else break;

        ignoreRemainingInput(); // Clear remaining input.
        readInput(std::cin); // Read user input.

        // Process scoring and end of turn options.
        if (score.getPermanentScore() >= 1000 || score.getRoundScore() >= 1000) {
            score.displayHighScoreInfo(currentPlayer->getName(), game.findHighestScoringPlayer()->getName());

            // Handle end turn decision.
            if (uint16_t playOrEndTurn = 0; enterEndTurnOption(playOrEndTurn)) {
                // Since enterEndTurnOption() returned true, the user chose to end the turn.
                if (playOrEndTurn == 2 && (score.getPermanentScore() >= 1000 || score.getRoundScore() >= 1000))
                    applyPossibleOptions();
                updateGameStatus(playOrEndTurn);
                clear(); // Clear screen.
            }

            // Handle scenario where no options are left.
        } else if (!isOptionAvailable())
            handleNoOptionsLeft();
    } while (game.getTurnContinuationStatus() && game.getSelectionContinuationStatus() && isOptionAvailable());
    // Loop as long as options available and turn continues.
}


void Checker::checkStraits() const
{
    if (!isStrait())
        return;

    game.getCurrentPlayer()->getScore().increaseRoundScore(1000);
    game.getCurrentPlayer()->getDice().diceSetMap.clear();
    game.setSelectedOptionStatus(true);
}

void Checker::checkSet() const
{
    if (!isSet())
        return;

    game.getCurrentPlayer()->getScore().increaseRoundScore(1000);
    game.getCurrentPlayer()->getDice().diceSetMap.clear();
    game.setSelectedOptionStatus(true);
}

void Checker::checkMultiple(const uint16_t dieValue) const
{
    // Variables
    Player* const& currentPlayer = game.getCurrentPlayer();
    Dice& dice = currentPlayer->getDice();
    Score& score = currentPlayer->getScore();

    ///   MULTIPLE   ///
    if (isMultiple() && dice.diceSetMap[dieValue] >= 3) {
        // Error Checking
        if (dice.diceSetMap[dieValue] > FULL_SET_OF_DICE)
            throw std::out_of_range(
                "Error: You have " + std::to_string(dice.diceSetMap[dieValue]) +
                " dice. \nYou cannot have more than 6 dice."
            );

        // Variables
        const uint16_t baseScore = (dieValue == 1) ? 1000 : dieValue * 100;
        const uint16_t numMultiples = dice.diceSetMap[dieValue] - 3;
        const auto mScore = static_cast<uint16_t>(pow(2, numMultiples) * baseScore);

        // Handle Multiples Found
        score.setScoreFromMultiples(mScore);
        score.increaseRoundScore(mScore);
        game.setValueOfChosenMultiple(dieValue);

        ///   ADDING MULTIPLE   ///
    } else if (canAddMultiples()) {
        // Error Checking
        if ((score.getScoreFromMultiples() < 200) || (dice.diceSetMap[dieValue] > 3))
            throw std::out_of_range(
                "Error: You must have had a multiple selected and cannot have more than 6 dice or 3 added multiples"
            );

        // Variables
        const auto mScore = static_cast<uint16_t>(pow(2, dice.diceSetMap[dieValue]) * score.getScoreFromMultiples());

        // Handle Adding Multiple
        score.increaseRoundScore(mScore - score.getScoreFromMultiples()); // Subtract to avoid double adding
        score.setScoreFromMultiples(mScore);
    } else return;

    // Multiple Updates (adding and regular)
    game.manageDiceCount(currentPlayer->getDice().getNumDiceInPlay() - dice.diceSetMap[dieValue]);
    dice.eliminateDice(dieValue);
    dice.calculateMultipleAvailability();
    game.setSelectedOptionStatus(true);
}

void Checker::checkSingle(const uint16_t dieValue) const
{
    // Error Checking
    if (!((dieValue == 1 && isSingle(dieValue)) || (dieValue == 5 && isSingle(dieValue))))
        return;

    // Variables
    Player* currentPlayer = game.getCurrentPlayer();
    Score& score = currentPlayer->getScore();

    // Force Multiple option if Available
    if (std::map<uint16_t, uint16_t>& diceSetMap = currentPlayer->getDice().diceSetMap;
        (game.getValueOfChosenMultiple() == dieValue && canAddMultiples()) || (
            diceSetMap[dieValue] >= 3 && isMultiple())) {
        std::cout << "The option to chose multiples has been automatically applied.\n" << std::endl;
        checkMultiple(dieValue);
        // Apply Singles if available
    } else if (1 <= diceSetMap[dieValue] && diceSetMap[dieValue] < 3) {
        score.increaseRoundScore((dieValue == 1) ? 100 : 50);
        diceSetMap[dieValue]--;
        game.manageDiceCount(currentPlayer->getDice().getNumDiceInPlay() - 1);
        game.setSelectedOptionStatus(true);
    }
}

void Checker::applyPossibleOptions() const
{
    while (isOptionAvailable()) {
        if (isStrait())
            checkStraits();
        if (isSet())
            checkSet();
        for (uint16_t dieValue = 1; dieValue <= FULL_SET_OF_DICE; dieValue++)
            if ((isMultiple() && isDesiredMultipleAvailable(dieValue)) || (
                (game.getCurrentPlayer()->getScore().getScoreFromMultiples() >= 200) && (
                    game.getValueOfChosenMultiple() == dieValue) && canAddMultiples()))
                checkMultiple(dieValue);

        if (isSingle(1))
            checkSingle(1);
        if (isSingle(5))
            checkSingle(5);
    }
}

bool Checker::isStrait() const
{
    std::map<uint16_t, uint16_t>& diceSetMap = game.getCurrentPlayer()->getDice().diceSetMap;
    uint16_t i = 0;
    return (diceSetMap.size() == FULL_SET_OF_DICE && std::ranges::all_of(
        diceSetMap, [&i](const auto& die) {
            return die.first == ++i && die.second == 1;
        }
    ));
}

bool Checker::isSet() const
{
    return (game.getCurrentPlayer()->getDice().diceSetMap.size() == 3 && !isStrait() && !isMultiple() &&
        std::ranges::all_of(
            game.getCurrentPlayer()->getDice().diceSetMap,
            [](const auto& die) { return die.second == 2; }
        ));
}

bool Checker::isMultiple() const
{
    return game.getCurrentPlayer()->getDice().isMultipleAvailable();
}

bool Checker::isDesiredMultipleAvailable(const uint16_t desiredMultiple) const
{
    return game.getCurrentPlayer()->getDice().diceSetMap[desiredMultiple] >= 3;
}

bool Checker::canAddMultiples() const
{
    return (std::ranges::any_of(
        game.getCurrentPlayer()->getDice().diceSetMap, [this](const auto& die) {
            return die.first == game.getValueOfChosenMultiple();
        }
    ));
}

bool Checker::isSingle(uint16_t single) const
{
    return ((single == 1 || single == 5) && std::ranges::any_of(
        game.getCurrentPlayer()->getDice().diceSetMap,
        [single](const auto& die) {
            return die.first == single;
        }
    ));
}

bool Checker::isOptionAvailable() const
{
    return (isStrait() || isSet() || isMultiple() || canAddMultiples() || isSingle(1) || isSingle(5));
}


/*******************************
*   INPUT HANDLING FUNCTIONS   *
*******************************/

void Checker::readInput(std::istream& input) const
{
    Player* currentPlayer = game.getCurrentPlayer();
    const Score& score = currentPlayer->getScore();

    char ch = '\0';
    uint16_t val = 0;

    // Keep reading input until we get a meaningful character
    do input >> ch;
    while (ch == ' ' || (!strchr("stel123456am0?", ch)));

    switch (ch) {
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6': val = ch - '0'; // Convert char digit to number
            if (isStrait() || isSet() || (isMultiple() && isDesiredMultipleAvailable(val))
                || ((score.getScoreFromMultiples() >= 200) && (game.getValueOfChosenMultiple() == val) && (
                    canAddMultiples()))
                || isSingle(val))
                executeSelection(val);
            else displayImpossibleOptionMessage();
            break;
        case 's': executeSecondaryCommand(input);
            break;
        case 'a': input >> ch;
            input.clear();
            input.ignore(std::numeric_limits<int>::max(), '\n');
            if (ch == 'l')
                applyPossibleOptions();

        // No break here, falls through to case 'm' intentionally
        case 'm': input >> val;
            if (canProcessMultiple(val))
                checkMultiple(val);
            else handleFailedMultiple(val);
            break;
        case '0': processZeroCommand();
            break;
        case '?': displayPossibleOptions();
            break;
        default: displayImpossibleOptionMessage();
            break;
    }
}

// Execute dice selection based on the value
void Checker::executeSelection(const uint16_t val) const
{
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
void Checker::executeSecondaryCommand(std::istream& input) const
{
    char ch;
    input >> ch;
    if ((ch == 't' && isStrait()) || (ch == 'e' && isSet()))
        executeSelection(ch);
    else if ((ch == '1' && isSingle(1)) || (ch == '5' && isSingle(5)))
        checkSingle(ch - '0');
    else displayImpossibleOptionMessage();
}

// Handles failed 'm' command
bool Checker::canProcessMultiple(const uint16_t val) const
{
    return (isMultiple() && isDesiredMultipleAvailable(val)) || (
        (game.getCurrentPlayer()->getScore().getScoreFromMultiples() >= 200) && (
            game.getValueOfChosenMultiple() == val) && (canAddMultiples()));
}

void Checker::handleFailedMultiple(const uint16_t chosenMultiple) const
{
    std::cout << "You have selected an impossible option" << std::endl;

    if (chosenMultiple) {
        for (const auto& fst : game.getCurrentPlayer()->getDice().diceSetMap | std::views::keys)
            if (isDesiredMultipleAvailable(fst))
                std::cout << "\tMultiples, value: " << fst << std::endl;
            else if (game.getValueOfChosenMultiple() == fst)
                std::cout << "Add to Multiple, value: " << fst << std::endl;
    } else {
        clear();
        std::cout << "There is no available multiple" << std::endl;
    }
}

// Handles '0' command
void Checker::processZeroCommand() const
{
    Score& score = game.getCurrentPlayer()->getScore();
    if (game.getSelectedOptionStatus())
        game.setSelectionContinuationStatus(false);
    else if ((score.getRoundScore() >= 1000) && isOptionAvailable()) {
        score.increasePermanentScore(score.getRoundScore());
        std::cout << "Your official score is now: " << score.getPermanentScore() << std::endl;
    } else if (!isOptionAvailable()) {
        clear();
    } else {
        clear();
        std::cout << "You cannot end without a score higher than 1000" << std::endl;
        pauseAndContinue();
    }
}

// Handles '?' command
void Checker::displayPossibleOptions() const
{
    std::cout << "Possible options: " << std::endl;
    if (isStrait())
        std::cout << "\tStrait" << std::endl;
    if (isSet())
        std::cout << "\tSets" << std::endl;
    if (isMultiple())
        for (const auto& fst : game.getCurrentPlayer()->getDice().diceSetMap | std::views::keys)
            if (isDesiredMultipleAvailable(fst))
                std::cout << "\tMultiples, value: " << fst << std::endl;
    if (canAddMultiples())
        std::cout << "\tAddons, value: " << game.getValueOfChosenMultiple() << std::endl;
    if (isSingle(1) || isSingle(5))
        std::cout << "\tSingles" << std::endl;
    pauseAndContinue();
}

// Display common impossible option message
void Checker::displayImpossibleOptionMessage()
{
    clear();
    std::cout << "You have selected an impossible option" << std::endl;
}


/*
bool dieExists(uin16_t dieValue) {
 return std::any_of(game.getCurrentPlayer()->getDice().diceSetMap.begin(), game.getCurrentPlayer()->getDice().diceSetMap.end(), [dieValue](const auto& die) { return die.first == dieValue; })
}
*/
