// Checker.h
/**
Author: Jacob Anderson
Class: ECE-6122-A
Last Date Modified: 12/1/2025

Description:
Header file for the Checker class, which provides methods to check available scoring options
based on the current state of the game managed by GameManager.
*/

#ifndef ZILCH_CHECKER_H
#define ZILCH_CHECKER_H

#include <functional>
#include <string>
#include <vector>

/**
 * @class GameManager GameManager.h "GameManager"
 */
class GameManager;

/**
 * @class Checker Checker.h "Checker.h"
 *
 * @brief Provides methods to check available scoring options based on the current game state.
 */
class Checker {
public:
	// Represents a scoring option with a label and associated action.
	struct Option {
		std::string label;
		std::function<void()> action;
	};

	// Constructor that takes a reference to the GameManager.
	explicit Checker(GameManager& game) : game(game) {}

	// Retrieves a list of available scoring options.
	[[nodiscard]] std::vector<Option> getAvailableOptions() const;
	void handleBusts() const;

	// Individual option checks.
	[[nodiscard]] bool isStrait() const;
	[[nodiscard]] bool isSet() const;
	[[nodiscard]] bool isMultiple() const;
	[[nodiscard]] bool isDesiredMultipleAvailable(uint16_t desiredMultiple) const;
	[[nodiscard]] bool canAddMultiples() const;
	[[nodiscard]] bool isSingle(uint16_t single) const;
	[[nodiscard]] bool isOptionAvailable() const;

private:
	// Reference to the GameManager instance.
	GameManager& game;

	// Option application methods.
	void applyStrait() const;
	void applySet() const;
	void applyMultiple(uint16_t dieValue) const;
	void applySingle(uint16_t dieValue) const;
};

#endif //ZILCH_CHECKER_H
