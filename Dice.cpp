// Dice.cpp
/**
Author: Jacob Anderson
Class: ECE-6122-A
Last Date Modified: 12/2/2025

Description:
	Implementation of the Dice class for managing dice rolls and states.
*/

#include "Dice.h"

#include <random>
#include <ranges>


/**
 * @brief Removes entries with zero count from the dice set map.
 *
 * @param diceSetMap The map of dice values to their counts.
 */
static void removeZeros(std::map<uint16_t, uint16_t>& diceSetMap)
{
	for (auto it = diceSetMap.begin(); it != diceSetMap.end();)
		if (it->second == 0)
			it = diceSetMap.erase(it);
		else ++it;
}

/**
 * @brief Rolls the dice and updates the dice set map and last roll values.
 */
void Dice::rollDice()
{
	diceSetMap.clear();
	multipleExists = false;
	lastRoll.assign(getNumDiceInPlay(), 0);

	// Reuse a generator instead of reseeding for each roll.
	thread_local std::mt19937 mt(std::random_device{}());
	std::uniform_int_distribution<int> dist(1, FULL_SET_OF_DICE);
	for (size_t i = 0; i < static_cast<size_t>(getNumDiceInPlay()); i++) {
		lastRoll[i] = static_cast<uint16_t>(dist(mt));
	}

	// Tally rolled values and detect multiples.
	for (const auto dieValue : lastRoll) {
		if (++diceSetMap[dieValue] >= 3)
			multipleExists = true;
	}
}

/**
 * @brief Gets the number of dice currently in play.
 *
 * @return The number of dice in play.
 */
[[nodiscard]] uint16_t Dice::getNumDiceInPlay() const
{
	return numDiceInPlay;
}

/**
 * @brief Sets the number of dice in play, capped at 6.
 *
 * @param numOfDice The desired number of dice in play.
 */
void Dice::setNumDiceInPlay(const uint16_t numOfDice)
{
	removeZeros(diceSetMap);

	// Clamp dice count to valid range.
	if (numOfDice > 6)
		numDiceInPlay = 6;
	else numDiceInPlay = numOfDice;
}

/**
 * @brief Calculates if any multiples (three or more of a kind) are available in the current dice set.
 */
void Dice::calculateMultipleAvailability()
{
	multipleExists = false;

	for (const unsigned short snd : diceSetMap | std::views::values)
		if (snd >= 3)
			multipleExists = true;
}

/**
 * @brief Checks if any multiples are available.
 *
 * @return True if multiples exist, false otherwise.
 */
bool Dice::isMultipleAvailable() const
{
	return multipleExists;
}

/**
 * @brief Eliminates all dice of the specified value from the dice set.
 *
 * @param dieValue The value of the dice to eliminate.
 */
void Dice::eliminateDice(const uint16_t dieValue)
{
	diceSetMap[dieValue] = 0;
	removeZeros(diceSetMap);
}

/**
 * @brief Recounts the number of dice currently in play based on the dice set map.
 */
[[maybe_unused]] void Dice::recountNumDiceInPlay()
{
	removeZeros(diceSetMap);

	numDiceInPlay = 0;
	for (const auto snd : diceSetMap | std::views::values)
		numDiceInPlay += snd;
}
