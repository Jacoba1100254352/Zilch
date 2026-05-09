// GameStateMachine.cpp
/**
Author: Jacob Anderson
Class: ECE-6122-A
Last Date Modified: 12/2/2025

Description:
This file contains the implementation of the GameStateMachine class, which manages the different states of the game.
*/

#include "GameStateMachine.h"

/**
 * @brief Returns a string description of the current game state.
 *
 * @return A string representing the current state.
 */
std::string GameStateMachine::describe() const
{
	switch (current) {
		case State::Intro: return "Intro";
		case State::Rules: return "Rules";
		case State::Setup: return "Setup";
		case State::PlayerTurn: return "PlayerTurn";
		case State::Rolling: return "Rolling";
		case State::Resolving: return "Resolving";
		case State::Switching: return "Switching";
		case State::GameOver: return "GameOver";
		default: return "Unknown";
	}
}
