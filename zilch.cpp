#include "zilch.h"

/******************
*   CONSTRUCTOR   *
******************/
//zilch::zilch (unsigned ndip, unsigned cpp, unsigned npm, unsigned vcm, unsigned vam, unsigned b50, unsigned ap, unsigned dsv, unsigned s, unsigned ps, unsigned sl )
//    : numOfDiceInPlay ( ndip ), currentPlayerPosition ( cpp ), numOfPrevMultiples ( npm ), valOfChosenMultiple ( vcm ), valOfAvailableMultiple (vam), bust50 ( b50 ), amountOfPlayers ( ap ), diceSetVector ( dsv ), turnScore ( s ), permanentScore ( ps ), scoreLimit ( sl )
//{}

/************************
*   ROLLING FUNCTIONS   *
************************/

unsigned zilch::roll()
{
    ///   Use random library to effectivly randomize the dice rolls   ///
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<> dist(1, 7); // Roll between 1 and 6 (exclusive 7)
    //auto die = [&]() { return dist(mt); };
    return static_cast<unsigned>(dist(mt));
}
void zilch::rollSixDice( zilch& game )
{
/*    ///   Only roll if it is a new turn   ///
    if ( game.getNumOfDiceInPlay() == 6 )*/

    // We need this to roll and resize no matter what
    game.diceSetVector = { roll(), roll(), roll(), roll(), roll(), roll() };

    ///   If no dice left, reset to 6   ///
    if ( game.getNumOfDiceInPlay() == 0 )
        game.setNumOfDiceInPlay(6);


    ///   Set up dice vector
    game.diceSetVector.resize (game.getNumOfDiceInPlay() ); // Resize the dice vector to the number of dice in play
    std::sort ( game.diceSetVector.begin(), game.diceSetVector.end() ); // Sort dice between 1 and 6

    std::cout << "You have " << game.getNumOfDiceInPlay() << " dice left" << std::endl;

    /// Run checks on what is possible to play   ///
    check(game);
}
void zilch::showDice(const zilch& game )
{
    ///   Print dice vector   ///
    for ( unsigned i = 0; i < game.getNumOfDiceInPlay(); i++ )
        if ( i != game.getNumOfDiceInPlay() - 1 )
            std::cout << game.diceSetVector.at(i) << ", ";
        else std::cout << game.diceSetVector.at(i) << std::endl;
}

/************************
*   CHECKING FUNCTION   *
************************/
void zilch::check( zilch& game ) //checks to verify all options are exhausted
{
    //   if no options are available: else:
    if ( !straitBool(game) && !setBool(game) && !multiplesBool(game) && !multiplesAddBool(game) && !singleBool(game) )
        if ( game.getNumOfDiceInPlay() == 6 && !bust50Bool(game) ) // if bust on first roll without previous busts on the first roll
        {
            std::cout << "You have busted on the first roll, try again" << std::endl;
            game.setTurnScore( 50 );
            pauseAndContinue(game, 1 ); // 1 displays current score
        }
        else if ( game.getNumOfDiceInPlay() >= 1 ) // bust
        {
            showDice(game);
            std::cout << "You have busted" << std::endl
                      << "It is the next players turn" << std::endl;
            game.setTurnScore( 0 );
            pauseAndContinue(game, 0 );
        }
        else // reset the dice for the next roll on the same turn set
        {
            std::cout << "You have a full set of dice now" << std::endl;
            game.setNumOfDiceInPlay( 6 );
            pauseAndContinue(game, 1 );
        }
    else checkingUserInput(game); // Checks for a well ended turn (Options are available)
}

/*******************************
*   UNUSED FINDING FUNCTIONS   *
*******************************/
void zilch::checkingUserInput( zilch& game )
{
    ///   Variable Defaults   ///

    ///   Comment   ///
    if (straitBool(game) || setBool(game) || multiplesBool(game) || singleBool(game) || multiplesAddBool(game))
    {
        unsigned numOptions = 0;
        updateValOfAvailableMultiples(game);

        ///   Comment   ///
        if (multiplesAddBool(game) && game.getNumOfPrevMultiples() > 0 )
            std::cout << "You can add to your multiple of " << game.getValOfChosenMultiple() << "!\n";
        std::cout << game.getPlayers() << "'s current score: " << game.getRunningScore() << std::endl;
        showDice(game);
        std::cout << "\tSingles -- s#, \n\tStrait -- st, \n\tSet -- se, \n\tMultiples -- m#, \n\tAdding to Multiples -- a#;" << std::endl
                  << "Enter the amount of options you wish to select, or type 0 to end your turn: ";

        ///   Comment   ///
        Repeat:
        std::cin >> numOptions;
        while ( std::cin.fail() )
        {
            // NOTE: Uncomment when system("clear"); works
            /*system ( "clear" );
            std::cout << game.getPlayers() << "'s current score: " << game.getRunningScore() << std::endl;
            game.showDice(game);
            std::cout << "\tSingles -- s#, \n\tStrait -- st, \n\tSet -- se, \n\tMultiples -- m#, \n\tAdding to Multiples -- a#;" << std::endl*/
            std::cout << "Enter the amount of options you wish to select, or type 0 to end your turn: ";
            std::cin.clear();
            std::cin.ignore ( 256, '\n' );
            std::cin >> numOptions;
        }

        ///   Comment   ///
        if ( numOptions != 0 ) //If turn not ended, the options will be taken into account and noted
        {
            unsigned playOrEndTurn = 0;
            unsigned numOptionsLeft = 0;
            numOptionsLeft = numOptions;
            std::cout << "Which would you like to take: ";
            for ( unsigned i = 0; i < numOptions; i++ )
            {
                std::cin.ignore( 256, '\n' );
                std::cin >> game;
                numOptionsLeft--;
                if ( numOptionsLeft >= 1 )
                {
                    showDice(game);
                    std::cout << game.getPlayers() << "'s current score: " << game.getRunningScore() << std:: endl;
                    std::cout << "\tSingles -- s#, \n\tStrait -- st, \n\tSet -- se, \n\tMultiples -- m#, \n\tAdding to Multiples -- a#;" << std::endl
                              << "Please enter your next choice: ";
                }
                else if (game.getRunningScore() >= 1000 )
                {
                    std::cout << "You have " << game.getNumOfDiceInPlay() << " dice left" << std::endl;
                    std::cout << game.getPlayers() << "'s current score: " << game.getRunningScore() << std:: endl;
                    std::cout << "Type 1 to keep playing or 0 to end your turn: ";
                    std::cin >> playOrEndTurn;
                    system ( "clear" );
                    if ( playOrEndTurn == 0 )
                        goto EndTurn;
                }
            }
        }
        else
        {
            if (game.getRunningScore() >= 1000 )
            {
                pauseAndContinue(game, 0 );
                EndTurn:
                game.setPermanentScore( game.getTurnScore() );
                if (game.getPermanentScore() >= game.getScoreLimit() )
                    std::cout << game.getPlayers() << " has gone over " << game.getScoreLimit() << ", everyone else has one last turn" << std::endl;
                std::cout << game.getPlayers() << "'s permanent score has been logged and is now: " << game.getPermanentScore() << std::endl;
                game.setTurnScore( 0 );
                game.setNumOfDiceInPlay( 6 );
                game.setCurrentPlayerPosition( 1 );
                std::cout << "It is now " << game.getPlayers() << "'s turn" << std::endl;
            }
            else
            {
                std::cout << "You are not allowed to end without a permanent score higher than 1000" << std::endl;
                pauseAndContinue(game, 1 );
                showDice(game);
                std::cout << "\tSingles -- s#, \n\tStrait -- st, \n\tSet -- se, \n\tMultiples -- m#, \n\tAdding to Multiples -- a#;" << std::endl
                          << "Please re-enter the amount of options you wish to select: ";
                goto Repeat;
            }
        }
    }
}


/*********************************
*   DUMMY CHECKING FUNCTIONS     *
*********************************/
bool zilch::straitBool( const zilch& game )
{
    unsigned b = 0;
    if (game.getNumOfDiceInPlay() == 6 )
        for (unsigned i = 0; i < game.getNumOfDiceInPlay(); i++ )
            if ( game.diceSetVector.at(i) == i + 1 )
                b++;

    return b == 6 && game.getNumOfDiceInPlay() == 6;
}
bool zilch::setBool( const zilch& game )
{
    return game.getNumOfDiceInPlay() == 6 && !straitBool(game) && !multiplesBool(game) &&
        game.diceSetVector.at(0) == game.diceSetVector.at(1) && game.diceSetVector.at(2) == game.diceSetVector.at(3) &&
        game.diceSetVector.at(4) == game.diceSetVector.at(5) && game.diceSetVector.at(1) != game.diceSetVector.at(2) &&
        game.diceSetVector.at(3) != game.diceSetVector.at(4);
}
///   Checks to verify that the Multiple that the User wants is available   ///
bool zilch::desiredMultipleAvailabilityBool ( const zilch& game, const unsigned desiredMultiple )
{
    unsigned numOfDesiredMultiple = 0;
    for (unsigned i = 0; i < game.getNumOfDiceInPlay(); i++ )
        if ( game.diceSetVector.at(i) == desiredMultiple )
            numOfDesiredMultiple++;

    return numOfDesiredMultiple >= 3;
}
///   Checks to verify that Multiples exist   ///
bool zilch::multiplesBool( const zilch& game )
{
    unsigned numOfMultiples = 0;
    for (unsigned i = 0; i < game.getNumOfDiceInPlay()-1; i++ )
        if ( game.diceSetVector.at(i) == game.diceSetVector.at( i + 1 ) )
            numOfMultiples++;

    return numOfMultiples >= 3;
}
bool zilch::multiplesAddBool( const zilch& game )
{
    if (game.getNumOfPrevMultiples() > 0 )
    {
        unsigned a = 0;
        for (unsigned i = 0; i < game.getNumOfDiceInPlay(); i++ )
            if (game.diceSetVector.at(i) == game.getValOfChosenMultiple() )
                a++;

        return a >= 1;
    }
    return false;
}
bool zilch::singleBool( const zilch& game )
{
    unsigned a1 = 0;
    unsigned a5 = 0;
    for (unsigned i = 0; i < game.getNumOfDiceInPlay(); i++ )
        if ( game.diceSetVector.at(i) == 1 )
            a1++;
        else if ( game.diceSetVector.at(i) == 5 )
            a5++;

    return a1 != 0 || a5 != 0;
}
bool zilch::bust50Bool( const zilch& game )
{
    return game.bust50;
}
bool zilch::winBool( const  zilch& game )
{
    for (unsigned i = 0; i < game.getAmountOfPlayers(); i++ )
        if (game.permanentScore.at(i) >= game.getScoreLimit())
            return true;
    return false;
}


/****************************************
*   CHECKING AND OUTPUTTING FUNCTIONS   *
****************************************/
void zilch::straits( zilch& game )
{
    if ( straitBool(game) )
    {
        system ( "clear" );
        std::cout << "You have taken the Strait" << std::endl << std::flush;
        game.setTurnScore( 1000 );
    }
}
void zilch::set( zilch& game )
{
    if ( setBool(game) )
    {
        system ( "clear" );
        std::cout << "You have taken the Set" << std::endl << std::flush;
        game.setTurnScore( 1000 );
    }
}
void zilch::multiple ( zilch& game, const unsigned VAL_OF_DESIRED_MULTIPLE )
{
    system ( "clear" );
    std::cout << "You have taken the Multiple" << std::endl;
    game.setNumOfPrevMultiples( 1 );
    game.setValOfChosenMultiple( VAL_OF_DESIRED_MULTIPLE ); // Passes the value of the multiple
    unsigned numOfEachDice[6] = { }; // initializes an array of 6 0s

    ///   Note: This is offset so as to refer to the position of the array but be understandable by the user   ///
    const unsigned DIE_VALUE = VAL_OF_DESIRED_MULTIPLE - 1; // used in the array, if val = die 1, then numOfEachDice[dieValue] refers to the first slot in the array

    /// Counts and stores the number of each dice in array numOfEachDice
    for (unsigned i = 0; i < game.getNumOfDiceInPlay(); i++ )
        for ( unsigned k = 0; k < 6; k++)
            if ( game.diceSetVector.at(i) == k )
                numOfEachDice[k]++;
            else
                throw std::out_of_range ( "You cannot have a die with a value greater than 6" );


    ///   Sets the turn score based on the number of Multiples selected
    switch (numOfEachDice[DIE_VALUE])
    {
        case 3:
        case 4: // 3 or 4 dice
            if (VAL_OF_DESIRED_MULTIPLE == 1)
                game.setTurnScore(1000);
            else game.setTurnScore((numOfEachDice[DIE_VALUE] - 2) * (VAL_OF_DESIRED_MULTIPLE * 100));
            break;

        case 5: // 5 dice
            game.setTurnScore(VAL_OF_DESIRED_MULTIPLE * 400);
            break;

        case 6: // 6 dice
            game.setTurnScore(VAL_OF_DESIRED_MULTIPLE * 800);
            break;

        default:
            std::cout << "Default case reached," << std::endl
                      << "Number of dice: " << numOfEachDice[DIE_VALUE] << std:: endl
                      << "Line: " << __LINE__ << std::endl;
            throw std::out_of_range("Error: Cannot have more than 6 dice or less than 3 multiples");
    }


    ///   Deletes the chosen multiple from theand resizes (sets to 7, sorts, and resizes)
    unsigned numMultiplesChosen = 0;
    for (unsigned i = 0; i < game.getNumOfDiceInPlay(); i++ )
        if (game.diceSetVector.at(i) == VAL_OF_DESIRED_MULTIPLE )
        {
            game.diceSetVector.at(i) = 7;
            numMultiplesChosen++;
        }

    sort( game.diceSetVector.begin(), game.diceSetVector.end() );
    game.diceSetVector.resize(game.getNumOfDiceInPlay() - numMultiplesChosen );
    game.setNumOfDiceInPlay(game.diceSetVector.size());
}
void zilch::multipleAdd ( zilch& game, unsigned valOfAddedMultiple )
{
    system ( "clear" );
    std::cout << "You have chosen to add to the Multiples" << std::endl;

    unsigned numOfEachDice[6] = { }; // initializes an array of 6 0s

    ///   Note: This is offset so as to refer to the position of the array but be understandable by the user   ///
    const unsigned DIE_VALUE = valOfAddedMultiple - 1; // used in the array, if val = die 1, then numOfEachDice[dieValue] refers to the first slot in the array

    /// Counts and stores the number of each dice in array numOfEachDice
    for (unsigned i = 0; i < game.getNumOfDiceInPlay(); i++ )
        for ( unsigned k = 0; k < 6; k++)
            if ( game.diceSetVector.at(i) == k )
                numOfEachDice[k]++;
            else
                throw std::out_of_range ( "You cannot have a die with a value greater than 6" );


    if ( valOfAddedMultiple == 1 )
        valOfAddedMultiple *= 10; // (same as valOfAddedMultiple = 10) changed so that the scoring would function correctly


    switch ( numOfEachDice[DIE_VALUE] )
    {
        case 1:
            if ( game.getNumOfPrevMultiples() == 1 )
                game.setTurnScore( valOfAddedMultiple * 100 );
            if ( game.getNumOfPrevMultiples() == 2 )
                game.setTurnScore( valOfAddedMultiple * 300 );
            if ( game.getNumOfPrevMultiples() == 3 )
                game.setTurnScore( valOfAddedMultiple * 700 );
            break;

        case 2:
            if ( game.getNumOfPrevMultiples() == 1 )
                game.setTurnScore( valOfAddedMultiple * 200 );
            if ( game.getNumOfPrevMultiples() == 2 )
                game.setTurnScore( valOfAddedMultiple * 600 );
            break;

        case 3:
            if ( game.getNumOfPrevMultiples() == 1 )
                game.setTurnScore( valOfAddedMultiple * 400 );
            break;

        default:
            std::cout << "You have reached the default Line: " << __LINE__ << std::endl;
            break;
    }

    ///   Deletes the added multiple from theand resizes
    for (unsigned long i = 0; i < game.getNumOfDiceInPlay(); i++ )
        if ( game.diceSetVector.at(i) == valOfAddedMultiple )
        {
            game.diceSetVector.erase(game.diceSetVector.begin() + static_cast<long>(i));
            break;
        }
}
void zilch::single ( zilch& game, unsigned val )
{
    ///    Verify option validity   ///
    if (singleBool(game) && ((val == 1 ) || (val == 5 ) ) )
    {
        ///   Count the amount of each value   ///
        unsigned a = 0;
        for (unsigned i = 0; i < game.getNumOfDiceInPlay(); i++ )
            if ( game.diceSetVector.at(i) == val )
                a++;

        ///   Automatically use multiples if available, calculate additional multiples, or use singles   ///
        if (multiplesAddBool(game) && game.getValOfChosenMultiple() == val && game.getNumOfPrevMultiples() > 0 )
            multipleAdd(game, val);
        else if ( a < 3 && a >= 1 )
        {
            system ( "clear" );
            std::cout << "You have taken the Single" << std::endl;
            if ( val == 1 )
                game.setTurnScore( 100 );
                // ReSharper disable once CppDFAConstantConditions
            else if ( val == 5 )
                game.setTurnScore( 50 );

            // Delete used dice
            for (unsigned i = 0; i < game.getNumOfDiceInPlay(); i++ )
                if ( game.diceSetVector.at(i) == val )
                {
                    game.diceSetVector.erase(game.diceSetVector.begin() + i);
                    game.setNumOfDiceInPlay(game.getNumOfDiceInPlay()-1);
                    break;
                }

        }
        else if ( a >= 3 )
            multiple(game, val);
    }
}

void zilch::updateValOfAvailableMultiples( zilch& game )
{
    unsigned numOfMultiples = 0;
    for (unsigned i = 0; i < game.getNumOfDiceInPlay()-1; i++ )
        if ( game.diceSetVector.at(i) == game.diceSetVector.at(i + 1) )
        {
            numOfMultiples++;
            if ( numOfMultiples >= 3 )
                game.setValOfAvailableMultiple( game.diceSetVector.at(i) );
        }
}


/********************************
*   ENDGAME AND TIE FUNCTIONS   *
********************************/
void zilch::lastTurnOpportunity( zilch& game, const unsigned FULL_SET_OF_DICE )
{
    ///   Initialize Variables   ///
    constexpr unsigned ADVANCE_TO_NEXT_PLAYER = 1; // used exclusively by setCurrentPlayerPosition to avoid confusion
    unsigned posOfPlayerWithHighestScore = 0;
    unsigned positionOfGameEndingPlayer = game.getCurrentPlayerPosition(); // sets to current player so that every other player has 1 more turn
    bool turnEnd = false;

    ///   Sets the first player (regardless of ties) that has the highest to variable posOfPlayerWithHighestScore   ///
    for ( unsigned playerPosition = 0; playerPosition < game.getAmountOfPlayers(); playerPosition++ )
        if (game.getPermanentScore( playerPosition ) > game.getPermanentScore( posOfPlayerWithHighestScore ) )
            posOfPlayerWithHighestScore = playerPosition;

    game.setCurrentPlayerPosition(ADVANCE_TO_NEXT_PLAYER); // Starts the round with the person to the left of the player who triggered the end (surpassed scoreLimit)
    std::cout << game.getPlayers( posOfPlayerWithHighestScore ) << " is over " << game.getScoreLimit() << "\nEveryone else has one more chance to win" << std::endl;

    for ( ; game.getCurrentPlayerPosition() != positionOfGameEndingPlayer; game.setCurrentPlayerPosition( ADVANCE_TO_NEXT_PLAYER ) )
        while ( !turnEnd )
        {
            if (game.getTurnScore() == 0 && game.getNumOfDiceInPlay() != FULL_SET_OF_DICE)
            {
                std::cout << "It is " << game.getPlayers() << "'s turn" << std::endl;
                game.setNumOfDiceInPlay(FULL_SET_OF_DICE);
                turnEnd = true;
            }
            else
            {
                rollSixDice(game);
                std::cout << game.getPlayers() << "'s Score: " << game.getRunningScore() << "\n" << std::endl;
            }
        }

    ///   Handles case of tying winners   ///
    tiedEnding(game, posOfPlayerWithHighestScore, positionOfGameEndingPlayer );
}

void zilch::tiedEnding( zilch& game, unsigned posOfPlayerWithHighestScore, unsigned positionOfGameEndingPlayer  )
{
    unsigned tieCounter = 0;
    unsigned highestScore = game.getPermanentScore( posOfPlayerWithHighestScore ); // Sets a variable with the highest score
    std::vector<unsigned> tie;
    tie.resize(game.getAmountOfPlayers() );
    tie.at(0) = { posOfPlayerWithHighestScore };

    ///   Compares each player with the Highest Scoring Players Score to check for a tie   ///
    for ( unsigned i = 0; i < game.getAmountOfPlayers(); i++ )
        if (game.getPermanentScore(i) == highestScore )
            tie.at(++tieCounter) = { i };

    ///   Outputs congratulations on condition of a tie or solo win   ///
    if ( tieCounter > 1 )
    {
        for ( unsigned i = 0; i < tieCounter; i++ )
        {
            std::cout << game.getPlayers(tie.at(i));
            if ( i != tieCounter - 1 )
                std::cout << ", ";
            else
                std::cout << " and ";
        }
        std::cout << " have tied with " << highestScore << " Points!" << std::endl;
    }
    else std::cout << game.getPlayers( positionOfGameEndingPlayer ) << " won with " << highestScore << " Points!" << std::endl;
    pauseAndContinue(game, 0);
}


/***************************
*   Aesthetics Functions   *
***************************/
void zilch::pauseAndContinue ( zilch& game, const unsigned val )
{
    std::cin.ignore ( 256, '\n' );
    std::cout << "\nPress enter to continue ...";
    std::cin.get();
    system ( "clear" );
    if ( val == 1 )
        std::cout << game.getPlayers() << "'s current score: " << game.getRunningScore() << std::endl;
}

/******************************
*   INPUT OPERATOR OVERLOAD   *
******************************/
std::istream& operator>> (std::istream& input, zilch& outputGame) // NOLINT(*-no-recursion)
{
    char ch = '\0';
    unsigned val = 0;

    input >> ch;


    while ( ch == ' ' || ch != 's' && ch != 't' && ch != 'e' && ch != '1' && ch != '5' && ch != 'm' && ch != 'a' && ch != '0' && ch != '?' )
        input >> ch;

    switch ( ch )
    {
        case 's':
            input >> ch;
            if (ch == 't' && zilch::straitBool(outputGame))
                zilch::straits(outputGame);
            else if (ch == 'e' && zilch::setBool(outputGame))
                zilch::set(outputGame);
            else
            {
                if (ch == '1' && zilch::singleBool(outputGame))
                    zilch::single(outputGame, 1);
                else if (ch == '5' && zilch::singleBool(outputGame))
                    zilch::single(outputGame, 5);
                else
                {
                    std::cout << "You have selected an impossible option, please select again" << std::endl
                              << "Please re-enter: ";
                    std::cin.ignore ( 256, '\n' );
                    std::cin >> outputGame;
                }
            }
            break;

        case 'a':
            input >> val;
            if (outputGame.getNumOfPrevMultiples() > 0 && outputGame.getValOfChosenMultiple() == val &&
                zilch::multiplesAddBool(outputGame))
                zilch::multipleAdd(outputGame, val);
            else
            {
                std::cout << "You cannot add on to a set multiples that you have not already taken" << std::endl
                          << "Pleas re-enter: ";
                std::cin.ignore ( 256, '\n' );
                std::cin >> outputGame;
            }
            break;

        case 'm':
            input >> val;
            if (zilch::multiplesBool(outputGame) && zilch::desiredMultipleAvailabilityBool(outputGame, val)) /// FIXME this should take care of it, but doesn't, look into it
                zilch::multiple(outputGame, val);
            else
            {
                std::cout << "The Value of the multiple is: " << outputGame.getValOfChosenMultiple() << std::endl;
                std::cout << "You have selected an impossible option" << std::endl
                          << "Please re-enter: ";
                std::cin.ignore ( 256, '\n' );
                std::cin  >> outputGame;
            }
            break;

        case '0':
            if (outputGame.getRunningScore() >= 1000 && ! (!zilch::straitBool(outputGame) && !zilch::setBool(outputGame)
                                                              && !zilch::multiplesBool(outputGame) && !zilch::multiplesAddBool(outputGame) && !zilch::singleBool(outputGame)) )
            {
                outputGame.setPermanentScore(outputGame.getTurnScore());
                outputGame.setTurnScore( 0 );
                std::cout << "Your official score is now: " << outputGame.getPermanentScore() << std::endl;
                outputGame.setNumOfDiceInPlay( 6 );
                outputGame.setCurrentPlayerPosition( 1 );
                std::cout << "It is now " << outputGame.getPlayers() << "'s turn" << std::endl;
            }
            else if (!zilch::straitBool(outputGame) && !zilch::setBool(outputGame) && !zilch::multiplesBool(outputGame)
                     && !zilch::multiplesAddBool(outputGame) && !zilch::singleBool(outputGame))
            {
                system( "clear" );
                return input;
            }
            else
            {
                unsigned c = 0;
                unsigned b = 0;
                std::cout << "You cannot end without a score higher than 1000" << std::endl;
                zilch::pauseAndContinue(outputGame, 1);
                zilch::showDice(outputGame);
                std::cout << "\tSingles -- s#, \n\tStrait -- st, \n\tSet -- se, \n\tMultiples -- m#, \n\tAdding to Multiples -- a#;" << std::endl
                          << "Please re-enter the amount of options you wish to select: ";
                std::cin >> b;
                while ( std::cin.fail() || ( b == 0 ) )
                {
                    system( "clear" );
                    std::cout << outputGame.getPlayers() << "'s current score: " << outputGame.getRunningScore() << std::endl;
                    zilch::showDice(outputGame);
                    std::cout << "\tSingles -- s#, \n\tStrait -- st, \n\tSet -- se, \n\tMultiples -- m#, \n\tAdding to Multiples -- a#;" << std::endl
                              << "Please re-enter the amount of options you wish to select: ";
                    std::cin.clear();
                    std::cin.ignore ( 256, '\n' );
                    std::cin >> b;
                }
                c = b;
                std::cout << "Which would you like to take: ";
                for ( unsigned i = 0; i < b; i++ )
                {
                    std::cin >> outputGame;
                    if ( c > 1 )
                    {
                        std::cout << "Please enter your next choice: ";
                        c--;
                    }
                }
            }
            break;

        case '?':
            std::cout << "Possible options: " << std::endl;
            if ( zilch::straitBool(outputGame) )
                std::cout << "\tStrait" << std::endl;
            if ( zilch::setBool(outputGame) )
                std::cout << "\tSets" << std::endl;
            if ( zilch::multiplesBool(outputGame) )
                std::cout << "\tMultiples, value: " << outputGame.getValOfAvailableMultiple() << std::endl;
            if ( zilch::multiplesAddBool(outputGame) )
                std::cout << "\tAddons, value: " << outputGame.getValOfChosenMultiple() << std::endl;
            if ( zilch::singleBool(outputGame) )
                std::cout << "\tSingles" << std::endl;

            std::cout << "\nPlease re-enter: ";
            std::cin.ignore ( 256, '\n' );
            std::cin >> outputGame;
            break;

        default:
            std::cout << "You have selected an impossible option" << std::endl
                      << "Please re-enter: ";
            std::cin.ignore ( 256, '\n' );
            std::cin >> outputGame;
            break;
    }

    return input;
}

/********************
*   END ZILCH.CPP   *
********************/
