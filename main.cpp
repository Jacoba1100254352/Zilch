#include "zilch.h"

/*
* Should I use 2-3 objects, one for general play variables, a list of objects for each player with their info? Maybe one for dice info?
*/

/*
* FIXME: When using get functions in for loops assign them to a variable to avoid multiple calls
*/

using namespace std;

int main()
{
    /**************************************
    *    Setting objects and variables    *
    **************************************/
    ///   Variable Defaults   ///
    constexpr unsigned FULL_SET_OF_DICE = 6;
    unsigned numPlayers = 2;
    unsigned scoreLimit = 5000;
    string player;
    zilch game;


    /***************************
    *    Setting up players    *
    ***************************/
    ///   Input score limit and failsafe   ///
    system ( "clear");
    cout << "Enter the desired score limit: ";
    cin >> scoreLimit;

    ///   Verify a valid input   ///
    while ( cin.fail() || scoreLimit < 1000 )
    {
        system ( "clear");
        cout << "Enter the desired score limit: ";
        cin.clear();
        cin.ignore ( 256, '\n' );
        cin >> scoreLimit;
    }


    ///   Input amount of players and failsafe   ///
    cout << "Enter the amount of people who are playing: ";
    cin >> numPlayers;

    ///   Verify a valid input   ///
    while ( cin.fail() || numPlayers == 0 )
    {
        system ( "clear");
        cout << "Enter the desired score limit: " << scoreLimit;
        cout << "\nEnter the amount of people who are playing: ";
        cin.clear();
        cin.ignore ( 256, '\n' );
        cin >> numPlayers;
    }


    ///   Initialize play   ///
    game.setScoreLimit(scoreLimit);             // Score limit
    game.setAmountOfPlayers(numPlayers);
    game.setCurrentPlayerPosition(0);
    game.setTurnScore(0);                   // Starting Turn Score of 0
    game.setPermanentScore(0);              // Starting Score of 0
    game.setNumOfDiceInPlay(FULL_SET_OF_DICE);  // Full set of dice


    ///   Input Players   ///
    for ( unsigned i = 0; i < numPlayers; i++ )
    {
        if ( i == 0 )
            cout << "Input your first player: "; // cout << "Input Player " << i << " : ";
        else
            cout << "Input your next player: ";

        cin >> player;
        cout << endl;
        game.setPlayers( i, player);
    }


    ///   Output Player Names and Scores   ///
    cout << "Your players are:" << endl;
    for ( unsigned i = 0; i < numPlayers; i++ )
        cout << game.getPlayers( i) << "\tStarting Score: " << game.getRunningScore() << endl;

    zilch::pauseAndContinue(game, 0);
    system("clear");

    /***********************************
    *    Rolling and Scoring people    *
    ***********************************/
    while ( !zilch::winBool(game) )
    {
        bool turnEnd = false;
        while ( !turnEnd )
        {
            if ( game.getTurnScore() == 0 && game.getNumOfDiceInPlay() != FULL_SET_OF_DICE)
            {
                game.setCurrentPlayerPosition( 1);
                cout << "It is " << game.getPlayers() << "'s turn" << endl;
                game.setNumOfDiceInPlay( FULL_SET_OF_DICE);
                turnEnd = true;
            }
            else // Roll dice and start game
            {
                zilch::rollSixDice(game);
                cout << game.getPlayers() << "'s Score: " << game.getRunningScore() << "\n\n";
            }
        }
        ///   Print all players' scores   ///
        for ( unsigned i = 0; i < numPlayers; i++ )
            cout << game.getPlayers( i ) << "\tCurrent Score: " << game.getPermanentScore( i ) << endl;
        zilch::pauseAndContinue(game, 0 );
    }


    ///   Last Turn, Tie, and Game End Function   ///
    zilch::lastTurnOpportunity(game, FULL_SET_OF_DICE );

    ///   Prints Ending Score of each player   ///
    for ( unsigned i = 0; i < numPlayers; i++ )
        cout << game.getPlayers( i) << "\tFinal Score: " << game.getPermanentScore( i) << endl;
    zilch::pauseAndContinue(game, 0 );

    return 0;
}
