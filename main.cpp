#include "Zilch.h"

int main() {
    // Create an instance of the ZilchGame class
    Zilch game;

    // Welcome message and rules
    game.printWelcomeMessage();

    // Set score limit
    game.inputScoreLimit();

    // Set number of players and their names
    game.setupPlayers();

    // Output player names and starting scores
    game.printPlayerScores();

    // Main game loop
    while (!game.isGameEnd())
        game.playTurn();

    // Print final scores
    game.printFinalScores();

    // Handle tie and end of the game
    game.handleTieAndGameEnd();

    return 0;
}
