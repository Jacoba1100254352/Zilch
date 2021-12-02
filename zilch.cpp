#include "zilch.h"

/************************
*   ROLLING FUNCTIONS   *
************************/
unsigned zilch::roll()
{
    ///   Use random library to effectively randomize the dice rolls   ///
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<> dist(1, 7); // Roll between 1 and 6 (exclusive 7)
    return static_cast<unsigned>(dist(mt));
}
void zilch::rollSixDice( zilch& game )
{
    zilch::clear();
    game.diceSetMap.clear(); // reset the map to be filled again
    game.setOptionSelectedBool(false); // No options have yet been selected after a new roll
    game.setContinueSelectingBool(true); // Reset to allow for continuous selection after each new roll

/*    ///   If no dice left, reset to 6   ///
    if ( game.getNumOfDiceInPlay() == 0 ) {
        game.setNumOfDiceInPlay( 6 );
    }*/ // This is done in Check()

    for (int i = 0; i < game.getNumOfDiceInPlay(); i++) {
        unsigned roll = game.roll();
        game.diceSetMap[roll]++;
    }

    std::cout << "\nThe dice have been rolled" << std::endl;

    /// Run checks on what is possible to play   ///
    zilch::check(game);
}

/************************
*   CHECKING FUNCTION   *
************************/
void zilch::check( zilch& game )
{
    //   if no options are available: else:
    if ( !availableOptionBool(game) )
        if ( game.getNumOfDiceInPlay() == 6 && !zilch::bust50Bool(game) && game.getScoreFromSingles() == 0 ) // if bust on first roll without previous busts on the first roll
        {
            std::cout << "You have busted on the first roll, try again" << std::endl;
            game.setScoreFromSingles(50);
            zilch::pauseAndContinue(game, 1 ); // 1 displays current score
        }
        else if ( game.getNumOfDiceInPlay() >= 1 ) // bust
        {
            zilch::showDice(game);
            std::cout << "You have busted" << std::endl;
            game.setNumOfDiceInPlay(0);
            game.setScoreFromSingles(0);
            zilch::pauseAndContinue(game, 0);
        }
        else // reset the dice for the next roll on the same turn set
        {
            std::cout << "You have a full set of dice now" << std::endl;
            game.setNumOfDiceInPlay( 6 );
            zilch::pauseAndContinue(game, 1 );
        }
    else zilch::checkingUserInput(game); // Checks for a well ended turn (Options are available)
}

/*******************************
*   UNUSED FINDING FUNCTIONS   *
*******************************/
void zilch::checkingUserInput( zilch& game )
{ // FIXME: the losing player can't chose "re-roll" instead they can only end???
    ///   Variable Defaults   ///
    unsigned playOrEndTurn = 0;

    ///   Handle Multiples   ///
    updateValOfAvailableMultiples(game);
    if (zilch::multiplesAddBool(game) && (game.getScoreFromMultiples() >= 200))
        std::cout << "You can add to your multiple of " << game.getValOfChosenMultiple() << "!\n";

    ///   Count Down for Options Chosen   ///
    do {

        ///   Output   ///
        if (!game.getOptionSelectedBool())
            zilch::printInstructions(game, ENTER);
        else if (zilch::availableOptionBool(game))
            zilch::printInstructions(game, NEXT);
        else break;

        ///   Input   ///
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<int>::max(), '\n');
        std::cin >> game;
        zilch::clear();

        ///   if: Score above 1000: check for highest player   ///
        if (game.getRunningScore() >= 1000) {
            ///   Calculate high score   ///
            unsigned highestScore = game.getPermanentScore(zilch::getHighestScoringPlayer(game));

            ///   if: High score below limit: Ask to end or keep playing   ///
            if (highestScore < game.getScoreLimit()) {
                std::cout << "\n" << game.getCurrentPlayer() << "'s current score: " << game.getRunningScore()
                          << "\nType 1 to keep playing or 0 to end your turn: ";
                std::cin >> playOrEndTurn;

                ///   Verify a valid input   ///
                while ( std::cin.fail() )
                {
                    zilch::clear();
                    std::cout << "Type 1 to keep playing or 0 to end your turn: ";
                    std::cin.clear();
                    std::cin.ignore ( std::numeric_limits<int>::max(), '\n' );
                    std::cin >> playOrEndTurn;
                }

                game.setContinueSelectingBool(playOrEndTurn != 0); // No need for goto now, if 0, set to false
                //system("clear");
            }

            ///   else if: Someone has surpassed the limit: try to beat them   ///
            else if (highestScore > game.getRunningScore()) { // If the current player is not the highest scoring player and is less than the highest score
                std::cout << "\n\nYour current score of " << game.getRunningScore() << " is " << highestScore - game.getRunningScore()
                          << " less than " << zilch::getHighestScoringPlayer(game) << "'s High Score of " << highestScore
                          << " so keep going! :)" << std::endl;
            }

            ///   else if: Tied for the highest   ///
            else if (zilch::getHighestScoringPlayer(game) != game.getCurrentPlayer()) {
                std::cout << "You are currently tied with the highest scoring player!" << std::endl
                          << "Type 1 to keep playing or 0 to end your turn: ";
                std::cin >> playOrEndTurn;

                ///   Verify a valid input   ///
                while ( std::cin.fail() )
                {
                    zilch::clear();
                    std::cout << "Type 1 to keep playing or 0 to end your turn: ";
                    std::cin.clear();
                    std::cin.ignore ( std::numeric_limits<int>::max(), '\n' );
                    std::cin >> playOrEndTurn;
                }

                game.setContinueSelectingBool(playOrEndTurn != 0); // No need for goto now, if 0, set to false
            }

            ///   else: You are the highest! Ask to end or keep playing   ///
            else {
                std::cout << "You are currently the highest scoring player" << std::endl
                          << "Type 1 to keep playing or 0 to end your turn: ";
                std::cin >> playOrEndTurn;

                ///   Verify a valid input   ///
                while ( std::cin.fail() )
                {
                    zilch::clear();
                    std::cout << "Type 1 to keep playing or 0 to end your turn: ";
                    std::cin.clear();
                    std::cin.ignore ( std::numeric_limits<int>::max(), '\n' );
                    std::cin >> playOrEndTurn;
                }

                game.setContinueSelectingBool(playOrEndTurn != 0); // No need for goto now, if 0, set to false
                //system("clear");
            }
        }

        ///   Below the limit without options (bust or re-roll)   ///
        else if (!zilch::availableOptionBool(game)) {
            zilch::clear();
            std::cout << "\nThere are no options left";
            break;
        }

        ///   Chose not to continue   ///
        if (!game.getContinueSelectingBool()) {

            ///   Greater than or equal to 1000: log and end turn   ///
            if (game.getRunningScore() >= 1000) {

                ///   Selective Aesthetic   ///
                if (playOrEndTurn != 0)
                    zilch::pauseAndContinue(game, 0);

                ///   Log and end turn   ///
                game.setPermanentScore(game.getTurnScore());
                std::cout << std::endl << game.getCurrentPlayer()
                          << "'s permanent score has been logged and is now: "
                          << game.getPermanentScore() << std::endl << std::endl;
                game.setNumOfDiceInPlay(0);
                break;
            }

            ///   else if: Option selected: roll again   ///
            else if (game.getOptionSelectedBool())  break;

            ///   else: No option selected and not above 1000: select an option   ///
            else {
                std::cout << "You are not allowed to end without a permanent score higher than 1000" << std::endl;
                game.setContinueSelectingBool(true);
                //zilch::pauseAndContinue(game, 1 );
                zilch::printInstructions(game, REENTER);
            }
        } // else, roll again
    } while (game.getContinueSelectingBool() && zilch::availableOptionBool(game));

    /// NOTE: From here it goes back to the while loop in main
}


/*********************************
*   DUMMY CHECKING FUNCTIONS     *
*********************************/
bool zilch::straitBool( const zilch& game )
{
    return (game.diceSetMap.size() == 6);
}
bool zilch::setBool( const zilch& game )
{
    return (game.diceSetMap.size() == 3 && !zilch::straitBool(game) && !zilch::multiplesBool(game) &&
            !std::any_of(game.diceSetMap.begin(), game.diceSetMap.end(), [](const auto &die) { return die.second != 2; }));
}
///   Checks to verify that the Multiple the User wants is available   ///
bool zilch::desiredMultipleAvailabilityBool ( const zilch& game, unsigned desiredMultiple )
{
    return ( std::any_of(game.diceSetMap.begin(), game.diceSetMap.end(), [desiredMultiple](const auto & die) { return die.first == desiredMultiple && die.second >= 3; }) );
}
///   Checks to verify that Multiples exist   ///
bool zilch::multiplesBool( const zilch& game )
{
    return ( std::any_of(game.diceSetMap.begin(), game.diceSetMap.end(), [](const auto & die) { return die.second >= 3; }) );
}
bool zilch::multiplesAddBool( const zilch& game )
{
    return ( std::any_of(game.diceSetMap.begin(), game.diceSetMap.end(), [&game](const auto & die) { return die.first == game.getValOfChosenMultiple(); }) );
}
bool zilch::singleBool( const zilch& game, unsigned single )
{
    return ( std::any_of(game.diceSetMap.begin(), game.diceSetMap.end(), [single](const auto & die) { return die.first == single; }) );
}
bool zilch::bust50Bool( const zilch& game )
{
    return (game.bust50 != 0);
}
bool zilch::winBool( zilch& game )
{
    for (unsigned i = 0; i < game.getAmountOfPlayers(); i++ )
        if (game.permanentScore[game.getCurrentPlayer()] >= game.getScoreLimit())
            return true;
    return false;
}
bool zilch::availableOptionBool( zilch& game )
{
    return zilch::straitBool(game) || zilch::setBool(game) || zilch::multiplesBool(game) || zilch::singleBool(game, 1) || zilch::singleBool(game, 5) || zilch::multiplesAddBool(game);
}


/****************************************
*   CHECKING AND OUTPUTTING FUNCTIONS   *
****************************************/
void zilch::straits( zilch& game )
{
    if ( !game.straitBool(game) )
        return;

    //system ( "clear" );
    game.setScoreFromSingles(1000);
    game.diceSetMap.clear();
    game.setOptionSelectedBool(true);
}
void zilch::set( zilch& game )
{
    game.getNumOfDiceInPlay();
    if ( !game.setBool(game) )
        return;

    //system ( "clear" );
    game.setScoreFromSingles(1000);
    game.diceSetMap.clear();
    game.setOptionSelectedBool(true);
}
void zilch::multiple ( zilch& game, const unsigned VAL_OF_DESIRED_MULTIPLE ) {
    if (!game.desiredMultipleAvailabilityBool(game, VAL_OF_DESIRED_MULTIPLE) && game.getValOfAvailableMultiple() == 0)
        return;

    if (multiplesBool(game)) {
        unsigned valFromPrevChosenMultiples = 0; /// This keeps track if a there are two possible sets example: 111666
        if (game.getValOfChosenMultiple() != 0)
            valFromPrevChosenMultiples = game.getScoreFromMultiples();
        game.setValOfChosenMultiple(VAL_OF_DESIRED_MULTIPLE); // Passes the value of the multiple
        const unsigned baseScore = (VAL_OF_DESIRED_MULTIPLE == 1) ? 1000 : VAL_OF_DESIRED_MULTIPLE * 100; // Sets the base score dependent on whether a 1 or another number passed
        const unsigned numMultiples = game.diceSetMap[VAL_OF_DESIRED_MULTIPLE] - 3; // When used as a power, having zero (2^0) produces 1 :)
        const auto score = static_cast<unsigned>(pow(2, numMultiples) * baseScore); // Calculates what is to be multiplied by the base score

        ///   Check for incorrect amount of dice/possible multiples   ///
        if (game.diceSetMap[VAL_OF_DESIRED_MULTIPLE] > 6 || game.diceSetMap[VAL_OF_DESIRED_MULTIPLE] - 3 < 0)
            throw std::out_of_range("Error: You have " + std::to_string(game.diceSetMap[VAL_OF_DESIRED_MULTIPLE]) + " dice. \nYou cannot have more than 6 dice or less than 3 multiples. \nLine: " + std::to_string(__LINE__));

        ///   Sets the turn score based on the number of Multiples selected   ///
        if (std::any_of(game.diceSetMap.begin(), game.diceSetMap.end(), [VAL_OF_DESIRED_MULTIPLE](const auto &die) { return die.first == VAL_OF_DESIRED_MULTIPLE; })) // Avoids inadvertently adding new value to the map
            game.setScoreFromMultiples(score);

        ///   This should solve the problem of the scores form two multiples sets (e.g. 111666) not adding   ///
        if (valFromPrevChosenMultiples > 0)
            game.setScoreFromMultiples(game.getScoreFromMultiples() + valFromPrevChosenMultiples);
    } else if (multiplesAddBool(game)) {

        ///   Check for the Correct number of Multiples   ///
        if ((game.getScoreFromMultiples() < 200) || (game.diceSetMap[VAL_OF_DESIRED_MULTIPLE] > 3))
            throw std::out_of_range("Error: You must have had a multiple selected and cannot have more than 6 dice or than 3 added multiples");

        ///   .   ///
        if (std::any_of(game.diceSetMap.begin(), game.diceSetMap.end(), [VAL_OF_DESIRED_MULTIPLE](const auto &die) { return die.first == VAL_OF_DESIRED_MULTIPLE; })) // Avoids inadvertently adding new value to the map
            game.setScoreFromMultiples(game.diceSetMap[VAL_OF_DESIRED_MULTIPLE]);
    } else {
        return;
    }

    ///   Gets rid of the chosen multiples   ///
    game.diceSetMap[VAL_OF_DESIRED_MULTIPLE] = 0;
    game.setNumOfDiceInPlay();
    game.setOptionSelectedBool(true);
}
void zilch::single ( zilch& game, unsigned val )
{
    ///    Verify option validity   ///
    if ( !((val == 1 && zilch::singleBool(game, val)) || (val == 5 && zilch::singleBool(game, 5))) )
        return;

    ///   Automatically use multiples if available, calculate additional multiples, or use singles   ///
    if ( (game.getValOfChosenMultiple() == val) || (game.diceSetMap[val] >= 3)) { // if (game.multiplesAddBool(game) && (game.getValOfChosenMultiple() == val ) )
        std::cout << "The option to chose multiples has been automatically applied.\n" << std::endl;
        game.multiple(game, val);
    } else if ( ( game.diceSetMap[val] < 3 ) && ( game.diceSetMap[val] >= 1 ) ) {
        //system ( "clear" );
        game.setScoreFromSingles((val == 1) ? 100 : 50); // Unless something really weird happens the other value will always be 5

        // Delete used dice
        game.diceSetMap[val]--;
        game.setNumOfDiceInPlay(-1);
        game.setOptionSelectedBool(true);
    }
}

void zilch::updateValOfAvailableMultiples( zilch& game )
{
    for (const auto & die : game.diceSetMap)
        if( die.second >= 3 )
            game.setValOfAvailableMultiple( die.first );
}


/********************************
*   ENDGAME AND TIE FUNCTIONS   *
********************************/
void zilch::lastTurnOpportunity( zilch& game, const unsigned FULL_SET_OF_DICE )
{
    ///   Initialize Variables   ///
    std::string gameEndingPlayer = game.getCurrentPlayer(); // sets to current player so that every other player has 1 more turn

    std::cout << gameEndingPlayer << " is over " << game.getScoreLimit() << std::endl
              << "Everyone else has one more chance to win" << std::endl << std::flush;
    zilch::pauseAndContinue(game, 0); // FIXME: This one also causes formatting issues
    std::cout << "\n" << std::endl;

    game.incCurrentPlayer();

    do {
        ///   Continue to next player   ///
        std::cout << "It is " << game.getCurrentPlayer() << "'s last turn" << std::endl;
        game.setNumOfDiceInPlay( FULL_SET_OF_DICE );

        while ((game.getTurnScore() != 0) || game.getNumOfDiceInPlay() == FULL_SET_OF_DICE) // TurnScore is 0 upon bust or start, and numDice is 6 at start
            game.rollSixDice(game);

        game.incCurrentPlayer();
    } while (game.getCurrentPlayer() != gameEndingPlayer );
}

void zilch::tiedEnding( zilch& game )
{
    ///   Variable Declarations   ///
    unsigned tieCounter = 0;
    std::string playerWithHighestScore = zilch::getHighestScoringPlayer(game);
    unsigned highestScore = game.getPermanentScore( playerWithHighestScore ); // Sets a variable with the highest score
    std::vector<std::string> tie;
    tie.resize(game.getAmountOfPlayers() );
    tie.at(0) = { playerWithHighestScore };

    ///   Compares each player with the Highest Scoring Players Score to check for a tie   ///
    for ( const auto & player : game.getPlayers())
        if (game.getPermanentScore(player) == highestScore )
            tie.at(++tieCounter) = { player };

    ///   Outputs congratulations on condition of a tie or solo win   ///
    if ( tieCounter > 1 )
    {
        for ( const auto & player : tie )
        {
            std::cout << player;
            if ( player != tie.at(tie.size() - 2) )
                std::cout << ", ";
            else if ( player != tie.at(tie.size() - 1) )
                std::cout << " and ";
        }
        std::cout << " have tied with " << highestScore << " Points!" << std::endl;
    }
    else std::cout << playerWithHighestScore << " won with " << highestScore << " Points!" << std::endl;
}


/***************************
*   Aesthetics Functions   *
***************************/
void zilch::pauseAndContinue ( zilch& game, unsigned val)
{
    std::cout << "\nPress enter to continue ..." << std::flush;
    //std::cin.get();
    if(strcmp(PLATFORM_NAME, "windows") > 0) {
        system("pause"); // For Windows users
        system("cls");
    }
    else if(strcmp(PLATFORM_NAME, "macos") > 0) {
        system("read"); // For  macOS users
        system("clear");

        if (!cur_term)
        {
            int result;
            setupterm( nullptr, STDOUT_FILENO, &result );
            //if (result <= 0) return;
        }
        putp( tigetstr( (char*)"clear" ) );
    }

    if ( val == 1 )
        std::cout << game.getCurrentPlayer() << "'s current score: " << game.getRunningScore() << std::endl;
}
void zilch::showDice( zilch& game )
{
    std::cout << "\nYou have " << game.getNumOfDiceInPlay() << " dice left" << std::endl
              << game.getCurrentPlayer() << "'s current score: " << game.getRunningScore() << std::endl;

    ///   Print dice vector   ///
    for (const auto & die : game.diceSetMap)
        for ( unsigned i = 0; i < die.second; i++ )
            if (die.first == game.diceSetMap.rbegin()->first && i == die.second - 1) // If it is on the very last element
                std::cout << die.first << std::endl;
            else std::cout << die.first << ", ";
}
void zilch::clear()
{
    if(strcmp(PLATFORM_NAME, "windows") > 0) {
        system("cls");
    }
    else if(strcmp(PLATFORM_NAME, "macos") > 0) {
        system("clear");

        if (!cur_term)
        {
            int result;
            setupterm( nullptr, STDOUT_FILENO, &result );
            //if (result <= 0) return;
        }
        putp( tigetstr( (char*)"clear" ) );
    }
}
void zilch::printInstructions( zilch& game, zilch::printOptions options )
{
    zilch::showDice(game);

    switch (options) {
        case zilch::ENTER:
            std::cout << "\tSingles -- s#, \n\tMultiples -- m#, \n\tSet -- se, \n\tStrait -- st;" << std::endl
                      << ((game.getOptionSelectedBool()) ? (game.getRunningScore() >= 1000)
                            ? "Enter the option you wish to take, or type 0 to end your turn: "
                            : "Enter the option you wish to take, or type 0 to roll again: "
                            : "Enter the option you wish to take: ");
            break;

        case zilch::NEXT:
            std::cout << "\tSingles -- s#, \n\tMultiples -- m#, \n\tSet -- se, \n\tStrait -- st;" << std::endl
                      << ((game.getOptionSelectedBool()) ? (game.getRunningScore() >= 1000)
                            ? "Please enter your next choice, or type 0 to end your turn: "
                            : "Please enter your next choice, or type 0 to roll again: "
                            : "Please enter your next choice: ");
            break;

        case zilch::REENTER:
            std::cout << "\tSingles -- s#, \n\tMultiples -- m#, \n\tSet -- se, \n\tStrait -- st;" << std::endl
                      << ((game.getOptionSelectedBool()) ? (game.getRunningScore() >= 1000)
                            ? "Please re-enter the option you wish to take, or type 0 to end your turn: "
                            : "Please re-enter the option you wish to take, or type 0 to roll again: "
                            : "Please re-enter the option you wish to take: ");
            break;

        default:
            throw std::invalid_argument("Invalid enumeration, Line: " + std::to_string(__LINE__));
            break;
    }
}

/******************************
*   INPUT OPERATOR OVERLOAD   *
******************************/
std::istream& operator>> (std::istream& input, zilch& game)
{
    char ch = '\0';
    unsigned val = 0;

    input >> ch;


    while ( ( ch == ' ' ) || ( ( ch != 's' ) && ( ch != 't' ) && ( ch != 'e' ) && ( ch != '1' ) && ( ch != '5' ) && ( ch != 'a' ) && ( ch != 'm' ) && ( ch != '0' ) && ( ch != '?' ) ) )
        input >> ch;

    switch ( ch )
    {
        case 's':
            input >> ch;
            if (( ch == 't' ) && game.straitBool(game))
                game.straits(game);
            else if (( ch == 'e' ) && game.setBool(game))
                game.set(game);
            else
            {
                if (( ch == '1' ) && game.singleBool(game, 1))
                    game.single(game, 1);
                else if (( ch == '5') && game.singleBool(game, 5))
                    game.single(game, 5);
                else {
                    game.clear();
                    std::cout << "You have selected an impossible option, please select again" << std::endl;
                }
            }
            break;

        case 'a':
        case 'm':
            input >> val;
            if ((game.multiplesBool(game) && game.desiredMultipleAvailabilityBool(game, val)) // Multiples available
            || ((game.getScoreFromMultiples() >= 200) && (game.getValOfChosenMultiple() == val) && (game.multiplesAddBool(game)))) // Adding multiples available
                game.multiple(game, val);
            else
            {
                std::cout << "You have selected an impossible option" << std::endl;
                zilch::updateValOfAvailableMultiples(game );
                if (game.getValOfChosenMultiple() != 0)
                    std::cout << "The Value of the multiple is: " << game.getValOfChosenMultiple() << std::endl;
                else if (game.getValOfAvailableMultiple() != 0)
                    std::cout << "The Value of the multiple is: " << game.getValOfAvailableMultiple() << std::endl;
                else {
                    game.clear();
                    std::cout << "There is no available multiple" << std::endl;
                }
            }
            break;

        case '0':
            if (game.getOptionSelectedBool())
                game.setContinueSelectingBool(false);
            else if ((game.getRunningScore() >= 1000 ) && game.availableOptionBool(game))
            {
                game.setPermanentScore(game.getScoreFromSingles());
                game.setScoreFromSingles(0);
                std::cout << "Your official score is now: " << game.getPermanentScore() << std::endl;
                game.setNumOfDiceInPlay(6 );
                game.incCurrentPlayer();
                std::cout << "\nIt is now " << game.getCurrentPlayer() << "'s turn" << std::endl;
            }
            else if (!game.availableOptionBool(game)) {
                zilch::clear();
                return input;
            }
            else {
                game.clear();
                std::cout << "You cannot end without a score higher than 1000" << std::endl;
                game.pauseAndContinue(game, 1);
            }
            break;

        case '?':
            std::cout << "Possible options: " << std::endl;
            if ( game.straitBool(game) )
                std::cout << "\tStrait" << std::endl;
            if ( game.setBool(game) )
                std::cout << "\tSets" << std::endl;
            if ( game.multiplesBool(game) )
                std::cout << "\tMultiples, value: " << game.getValOfAvailableMultiple() << std::endl;
            if ( game.multiplesAddBool(game) )
                std::cout << "\tAddons, value: " << game.getValOfChosenMultiple() << std::endl;
            if (game.singleBool(game, 1) || game.singleBool(game, 5) )
                std::cout << "\tSingles" << std::endl;
            break;

        default:
            game.clear();
            std::cout << "You have selected an impossible option" << std::endl;
            break;
    }

    return input;
}

/********************
*   END ZILCH.CPP   *
********************/
