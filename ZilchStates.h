// ZilchStates.h

#ifndef ZILCH_STATES_H
#define ZILCH_STATES_H

/**
 * The enum represents the major states the game can be in.
 */
enum class ZilchGameState {
	SETUP,           // Enter players, set score limit, etc.
	BEGIN_TURN,      // Prepare a player's new turn
	ROLL_DICE,       // Player rolls dice
	CHECK_OPTIONS,   // Check available scoring options
	SELECT_OPTION,   // Player chooses how to score or proceed
	APPLY_SCORE,     // Apply the chosen score to the player
	END_TURN,        // End the current player's turn
	CHECK_GAME_END,  // Check if the game has been won
	LAST_TURN,       // Handle "last turn" logic if someone exceeds limit
	GAME_OVER        // The game is finished
};

#endif // ZILCH_STATES_H
