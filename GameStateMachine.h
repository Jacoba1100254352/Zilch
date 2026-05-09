// GameStateMachine.h
/**
Author: Jacob Anderson
Class: ECE-6122-A
Last Date Modified: 12/2/2025

Description:
This file contains the declaration of the GameStateMachine class, which manages the different states of the game.
*/

#ifndef ZILCH_GAME_STATE_MACHINE_H
#define ZILCH_GAME_STATE_MACHINE_H

#include <string>

/**
 * @class GameStateMachine GameStateMachine.h "GameStateMachine.h"
 *
 * @brief Manages the different states of the game.
 */
class GameStateMachine {
public:
	/// Enumeration of possible game states.
	enum class State {
		Intro,
		Rules,
		Setup,
		PlayerTurn,
		Rolling,
		Resolving,
		Switching,
		GameOver
	};

	/// Constructor initializes the game state to Intro.
	GameStateMachine() : current(State::Intro) {}

	/// @brief Gets the current game state.
	[[nodiscard]] State getState() const { return current; }
	void setState(const State newState) { current = newState; }

	/// @brief Returns a string description of the current game state.
	[[nodiscard]] std::string describe() const;

private:
	State current;
};

#endif // ZILCH_GAME_STATE_MACHINE_H
