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

///   OTHER   ///
void Checker::ignoreRemainingInput()
{
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<int>::max(), '\n');
}

/************
*   CHECK   *
************/

void Checker::check() const
{
    Player* const& currentPlayer = game.getCurrentPlayer();
    const Dice& dice = currentPlayer->getDice();
    const Score& score = currentPlayer->getScore();

    // Prepare for user input
    // clear();
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
    Score& score = currentPlayer->getScore();

    do {
        clear(); // Clear screen.

        // Print instructions based on game status
        if (!game.getSelectedOptionStatus())
            printInstructions(game, GameManager::ENTER);
        else if (isOptionAvailable())
            printInstructions(game, GameManager::NEXT);
        else
            break;

        // Show a single main menu of possible moves
        displayMenuOptions();

        // Read user choice
        uint16_t choice = 0;
        std::cin >> choice;
        ignoreRemainingInput(); // Clear any leftover input

        switch (choice) {
            case 1: // Strait
                if (isStrait()) {
                    checkStraits();
                } else {
                    displayImpossibleOptionMessage();
                }
                break;
            case 2: // Set
                if (isSet()) {
                    checkSet();
                } else {
                    displayImpossibleOptionMessage();
                }
                break;
            case 3: { // Multiple
                uint16_t val = 0;
                std::cout << "Enter the die value you'd like to use for a multiple (1..6): ";
                std::cin >> val;
                ignoreRemainingInput();
                if ((val >= 1 && val <= 6 && (isMultiple() && isDesiredMultipleAvailable(val))) ||
                    ((score.getScoreFromMultiples() >= 200) &&
                     game.getValueOfChosenMultiple() == val && canAddMultiples()))
                {
                    checkMultiple(val);
                } else {
                    displayImpossibleOptionMessage();
                }
                break;
            }
            case 4: // Single 1
                if (isSingle(1)) {
                    checkSingle(1);
                } else {
                    displayImpossibleOptionMessage();
                }
                break;
            case 5: // Single 5
                if (isSingle(5)) {
                    checkSingle(5);
                } else {
                    displayImpossibleOptionMessage();
                }
                break;
            case 6: // End turn
            {
                // Only allowed if roundScore >= 1000 or permanentScore >= 1000
                if ((score.getRoundScore() >= 1000 || score.getPermanentScore() >= 1000) &&
                    isOptionAvailable())
                {
                    // Increase permanent score and end turn
                    score.increasePermanentScore(score.getRoundScore());
                    std::cout << "Your official score is now: " << score.getPermanentScore() << std::endl;
                    game.setTurnContinuationStatus(false);
                } else if (!isOptionAvailable()) {
                    // If no options left, end turn
                    clear();
                    game.setTurnContinuationStatus(false);
                } else {
                    clear();
                    std::cout << "You cannot end without a score of at least 1000\n";
                    pauseAndContinue();
                }
                break;
            }
            case 7: // Roll again
                // If the user has selected an option previously, we stop selecting further
                if (game.getSelectedOptionStatus()) {
                    game.setSelectionContinuationStatus(false);
                } else {
                    // Trying to roll again with no selected scoring is only valid if no option is found
                    if (!isOptionAvailable()) {
                        clear();
                    } else {
                        clear();
                        std::cout << "You must select at least one option before rolling again (or have no options)\n";
                        pauseAndContinue();
                    }
                }
                break;
            default:
                displayImpossibleOptionMessage();
                break;
        }

        // If no options are left to pick (and we haven't ended), handle bust logic
        if (!isOptionAvailable() && game.getTurnContinuationStatus()) {
            if (currentPlayer->getDice().getNumDiceInPlay() == FULL_SET_OF_DICE &&
                !score.getRoundScore()) {
                handleFirstRollBust();
            }
            else if (currentPlayer->getDice().getNumDiceInPlay() >= 1) {
                handleBust();
            }
            else {
                std::cout << "\nYou have a full set of dice now" << std::endl;
                pauseAndContinue();
                game.manageDiceCount(FULL_SET_OF_DICE);
            }
        }

    } while (game.getTurnContinuationStatus() &&
             game.getSelectionContinuationStatus() &&
             isOptionAvailable());
    // Loop as long as the turn continues, the user can still select, and options remain.
}

/*******************************
*   MENU / DISPLAY FUNCTIONS   *
*******************************/

void Checker::displayMenuOptions()
{
    std::cout << "\n=== Select an Option ===\n";
    std::cout << "1) Take a strait (if available)\n";
    std::cout << "2) Take a set (if available)\n";
    std::cout << "3) Take/add multiples (3 or more of a kind)\n";
    std::cout << "4) Take single 1 (if available)\n";
    std::cout << "5) Take single 5 (if available)\n";
    std::cout << "6) End turn (only valid if >= 1000 points)\n";
    std::cout << "7) Roll again\n";
    std::cout << "Choice: ";
}

void Checker::displayImpossibleOptionMessage()
{
    clear();
    std::cout << "You have selected an impossible or unavailable option." << std::endl;
    pauseAndContinue();
}

/*******************************
*         SCORING LOGIC        *
*******************************/

void Checker::checkStraits() const
{
    if (!isStrait()) return;
    game.getCurrentPlayer()->getScore().increaseRoundScore(1000);
    game.getCurrentPlayer()->getDice().diceSetMap.clear();
    game.setSelectedOptionStatus(true);
}

void Checker::checkSet() const
{
    if (!isSet()) return;
    game.getCurrentPlayer()->getScore().increaseRoundScore(1000);
    game.getCurrentPlayer()->getDice().diceSetMap.clear();
    game.setSelectedOptionStatus(true);
}

void Checker::checkMultiple(const uint16_t dieValue) const
{
    Player* const& currentPlayer = game.getCurrentPlayer();
    Dice& dice = currentPlayer->getDice();
    Score& score = currentPlayer->getScore();

    if (isMultiple() && dice.diceSetMap[dieValue] >= 3) {
        // Check for dice overflow
        if (dice.diceSetMap[dieValue] > FULL_SET_OF_DICE)
            throw std::out_of_range(
                "Error: " + std::to_string(dice.diceSetMap[dieValue]) +
                " dice cannot exist. Max is 6."
            );

        const uint16_t baseScore = (dieValue == 1) ? 1000 : dieValue * 100;
        const uint16_t numMultiples = dice.diceSetMap[dieValue] - 3;
        const auto mScore = static_cast<uint16_t>(std::pow(2, numMultiples) * baseScore);

        score.setScoreFromMultiples(mScore);
        score.increaseRoundScore(mScore);
        game.setValueOfChosenMultiple(dieValue);

    } else if (canAddMultiples()) {
        // Check for dice overflow
        if (score.getScoreFromMultiples() < 200 || (dice.diceSetMap[dieValue] > 3))
            throw std::out_of_range(
                "Error: Multiple selection invalid."
            );

        const auto mScore = static_cast<uint16_t>(
            std::pow(2, dice.diceSetMap[dieValue]) * score.getScoreFromMultiples()
        );

        score.increaseRoundScore(mScore - score.getScoreFromMultiples());
        score.setScoreFromMultiples(mScore);
    } else return;

    // Multiple Updates (adding and regular)
    game.manageDiceCount(dice.getNumDiceInPlay() - dice.diceSetMap[dieValue]);
    dice.eliminateDice(dieValue);
    dice.calculateMultipleAvailability();
    game.setSelectedOptionStatus(true);
}

void Checker::checkSingle(const uint16_t dieValue) const
{
    if (!((dieValue == 1 && isSingle(dieValue)) ||
          (dieValue == 5 && isSingle(dieValue))))
        return;

    Player* currentPlayer = game.getCurrentPlayer();
    Score& score = currentPlayer->getScore();

    // If we can or must use the Multiple option automatically
    if (auto& diceSetMap = currentPlayer->getDice().diceSetMap; (game.getValueOfChosenMultiple() == dieValue && canAddMultiples()) ||
        (diceSetMap[dieValue] >= 3 && isMultiple()))
    {
        std::cout << "Automatically applying Multiple selection.\n";
        checkMultiple(dieValue);
    }
    else if (diceSetMap[dieValue] > 0 && diceSetMap[dieValue] < 3) {
        score.increaseRoundScore((dieValue == 1) ? 100 : 50);
        diceSetMap[dieValue]--;
        game.manageDiceCount(currentPlayer->getDice().getNumDiceInPlay() - 1);
        game.setSelectedOptionStatus(true);
    }
}

/*******************************
*       HELPER FUNCTIONS       *
*******************************/

bool Checker::isStrait() const
{
    std::map<uint16_t, uint16_t>& diceSetMap = game.getCurrentPlayer()->getDice().diceSetMap;
    if (diceSetMap.size() != FULL_SET_OF_DICE) return false;

    uint16_t i = 0;
    return std::ranges::all_of(
        diceSetMap,
        [&i](const auto& die) {
            return die.first == ++i && die.second == 1;
        }
    );
}

bool Checker::isSet() const
{
    auto& diceSetMap = game.getCurrentPlayer()->getDice().diceSetMap;
    return diceSetMap.size() == 3 && !isStrait() && !isMultiple() &&
        std::ranges::all_of(
            diceSetMap,
            [](const auto& die) {
                return die.second == 2;
            }
        );
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
    return std::ranges::any_of(
        game.getCurrentPlayer()->getDice().diceSetMap,
        [this](const auto& die) {
            return die.first == game.getValueOfChosenMultiple();
        }
    );
}

bool Checker::isSingle(uint16_t single) const
{
    return (single == 1 || single == 5) && std::ranges::any_of(
        game.getCurrentPlayer()->getDice().diceSetMap,
        [single](const auto& die) {
            return die.first == single;
        }
    );
}

bool Checker::isOptionAvailable() const
{
    return isStrait() ||
           isSet() ||
           isMultiple() ||
           canAddMultiples() ||
           isSingle(1) ||
           isSingle(5);
}
