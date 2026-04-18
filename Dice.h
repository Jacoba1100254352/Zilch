// Dice.h
/**
Author: Jacob Anderson
Class: ECE-6122-A
Last Date Modified: 12/1/2025

Description:
Header for `Dice` class that manages dice rolls, counts, eliminations, and detection of multiples.
*/

#ifndef ZILCH_DICE_H
#define ZILCH_DICE_H

#include <map>
#include <vector>

#define FULL_SET_OF_DICE 6

/**
 * @class Dice Dice.h "Dice.h"
 *
 * @brief Manages dice rolls, counts, eliminations, and detection of multiples.
 */
class Dice {
public:
	// Constructor initializes with full set of dice.
	Dice() : numDiceInPlay(FULL_SET_OF_DICE), multipleExists(false) {}

	// Map of die value to count.
	std::map<uint16_t, uint16_t> diceSetMap;
	[[nodiscard]] const std::vector<uint16_t>& getLastRoll() const { return lastRoll; }

	// Rolls the dice and updates the dice set map and last roll values.
	void rollDice();

	// Gets the number of dice currently in play.
	[[nodiscard]] uint16_t getNumDiceInPlay() const;
	void setNumDiceInPlay(uint16_t numOfDice);

	// Recounts the number of dice in play based on the dice set map.
	[[maybe_unused]] void recountNumDiceInPlay();
	void eliminateDice(uint16_t dieValue);

	// Calculates if any multiples (three or more of a kind) are available in the current dice set.
	void calculateMultipleAvailability();
	[[nodiscard]] bool isMultipleAvailable() const;

private:
	uint16_t numDiceInPlay;
	bool multipleExists;
	std::vector<uint16_t> lastRoll;
};

#endif //ZILCH_DICE_H
