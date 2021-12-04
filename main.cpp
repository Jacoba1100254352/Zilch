#include "zilch.h"

/*
* Should I use 2-3 objects, one for general play variables, a list of objects for each player with their info? Maybe one for dice info?
*/

/*
* When using get functions in for loops assign them to a variable to avoid multiple calls
*/

using namespace std;

int main()
{
    /**************************************
    *    Setting objects and variables    *
    **************************************/
    ///   Variable Defaults   ///
    const unsigned FULL_SET_OF_DICE = 6;
    unsigned numPlayers = 2;
    unsigned scoreLimit = 5000;
    string player;
    zilch game;


    /***************************
    *    Setting up players    *
    ***************************/
    ///   Input score limit and failsafe   ///
    zilch::clear();
    cout << "Enter the desired score limit: " << flush;
    cin >> scoreLimit;

    ///   Verify a valid input   ///
    while ( cin.fail() || scoreLimit < 1000 )
    {
        zilch::clear();
        cout << "Enter the desired score limit: ";
        cin.clear();
        cin.ignore ( numeric_limits<int>::max(), '\n' );
        cin >> scoreLimit;
    }


    ///   Input amount of players and failsafe   ///
    cout << "Enter the amount of people who are playing: " << flush;
    cin >> numPlayers;

    ///   Verify a valid input   ///
    while ( cin.fail() || numPlayers == 0 )
    {
        system ( "clear");
        cout << "Enter the desired score limit: " << scoreLimit;
        cout << "\nEnter the amount of people who are playing: ";
        cin.clear();
        cin.ignore ( numeric_limits<int>::max(), '\n' );
        cin >> numPlayers;
    }


    ///   Initialize play   ///
    game.setScoreLimit(scoreLimit);                   // Score limit
    game.setAmountOfPlayers(numPlayers);
    game.setNumOfDiceInPlay(FULL_SET_OF_DICE);  // Full set of dice


    ///   Input Players   ///
    for ( unsigned i = 0; i < numPlayers; i++ )
    {
        ( i == 0 ) ? cout << "\nInput your first player: " << flush : cout << "Input your next player: " << flush;
        cin >> player;

        game.setPlayer( i, player);
        game.setTurnScores(0);                  // Each Player has a Starting Turn, Singles, and Multiples Score of 0
        game.setPermanentScore(0, true);   // Each Player has a Starting Score of 0
    }
    game.setCurrentPlayer(game.getPlayer(0));
    game.initializeMaps();


    ///   Output Player Names and Scores   ///
    cout << "\nYour players are:" << endl;
    for ( unsigned i = 0; i < numPlayers; i++ )
        cout << game.getPlayer( i) << "\tStarting Score: " << game.getRunningScore() << endl << flush;

    zilch::pauseAndContinue(game );
    zilch::clear();

    /***********************************
    *    Rolling and Scoring people    *
    ***********************************/
    while ( !game.winBool() )
    {
        game.setContinueTurnBool(true);
        while (game.getContinueTurnBool() && (game.getPermanentScore(game.getCurrentPlayer()) < game.getScoreLimit() ))
            zilch::rollSixDice(game);

        ///   Player Exceeded Score Limit   ///
        if (game.getPermanentScore(game.getCurrentPlayer()) >= scoreLimit)
            break;

        ///   Continue to next player   ///
        game.incCurrentPlayer(); // Also sets continueTurnBool to true
        cout << "\nIt is " << game.getCurrentPlayer() << "'s turn" << endl;
        game.setNumOfDiceInPlay( FULL_SET_OF_DICE);

        ///   Print all players' scores   ///
        for ( unsigned i = 0; i < numPlayers; i++ )
            cout << game.getPlayer( i ) << "\tCurrent Score: " << game.getPermanentScore( game.getPlayer( i ) ) << endl << flush;
        zilch::pauseAndContinue( game );
    }

    ///   Last Turn Function   ///
    zilch::lastTurnOpportunity(game, FULL_SET_OF_DICE );

    ///   Prints Ending Score of each player   ///
    for ( unsigned i = 0; i < numPlayers; i++ )
        cout << game.getPlayer( i) << "\tFinal Score: " << game.getPermanentScore( game.getPlayer( i )) << endl;

    ///   Tie and Game End Function   ///
    zilch::tiedEnding(game );

    return 0;
}
