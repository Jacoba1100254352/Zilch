#ifndef ZILCH_ZILCH_H
#define ZILCH_ZILCH_H

#include <iostream>
#include <algorithm>
#include <set>
#include <stdexcept>
#include <cstdlib>
#include <ctime>
#include <random>
#include <cmath>
#include <map>
#include <utility>
#include <vector>
#include <climits>
#include <unistd.h>
#include <term.h>

#if defined(_WIN32)
    #define PLATFORM_NAME "windows" // Windows
#elif defined(_WIN64)
    #define PLATFORM_NAME "windows" // Windows
#elif defined(__CYGWIN__) && !defined(_WIN32)
    #define PLATFORM_NAME "windows" // Windows (Cygwin POSIX under Microsoft Window)
#elif defined(__ANDROID__)
    #define PLATFORM_NAME "android" // Android (implies Linux, so it must come first)
#elif defined(__linux__)
    #define PLATFORM_NAME "linux" // Debian, Ubuntu, Gentoo, Fedora, openSUSE, RedHat, Centos and other
#elif defined(__unix__) || !defined(__APPLE__) && defined(__MACH__)
    #include <sys/param.h>
    #if defined(BSD)
        #define PLATFORM_NAME "bsd" // FreeBSD, NetBSD, OpenBSD, DragonFly BSD
    #endif
#elif defined(__hpux)
    #define PLATFORM_NAME "hp-ux" // HP-UX
#elif defined(_AIX)
    #define PLATFORM_NAME "aix" // IBM AIX
#elif defined(__APPLE__) && defined(__MACH__) // Apple OSX and iOS (Darwin)
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR == 1
        #define PLATFORM_NAME "ios" // Apple iOS
    #elif TARGET_OS_IPHONE == 1
        #define PLATFORM_NAME "ios" // Apple iOS
    #elif TARGET_OS_MAC == 1
        #define PLATFORM_NAME "osx" // Apple OSX
    #else
        #define PLATFORM_NAME "macos" // Apple OS
    #endif
#elif defined(__sun) && defined(__SVR4)
    #define PLATFORM_NAME "solaris" // Oracle Solaris, Open Indiana
#else
    #define PLATFORM_NAME NULL
#endif

class zilch
{
public:

    /************************
    *   ENUM FOR PRINTING   *
    ************************/
    enum printOptions{ ENTER, NEXT, REENTER };

    /************************
    *   ROLLING FUNCTIONS   *
    ************************/
    static unsigned roll() ;
    static void rollSixDice( zilch& ) ;

    /*******************************
    *   DUMMY CHECKING FUNCTIONS   *
    *******************************/
    static bool straitBool( const zilch& ) ;
    static bool setBool( const zilch& ) ;
    static bool desiredMultipleAvailabilityBool ( const zilch&, unsigned ) ; // Previously mValPass
    static bool multiplesBool( const zilch& ) ;
    static bool multiplesAddBool( const zilch& ) ;
    static bool singleBool( const zilch&, unsigned ) ;
    static bool bust50Bool( const zilch& ) ;
    static bool winBool( zilch& ) ;
    static bool availableOptionBool( zilch& );

    /****************************************
    *   CHECKING AND OUTPUTTING FUNCTIONS   *
    ****************************************/
    static void straits( zilch& ) ;
    static void set( zilch& ) ;
    static void multiple ( zilch&, unsigned ) ;
    static void single ( zilch&, unsigned ) ;
    static void check( zilch& ) ;

    static void updateValOfAvailableMultiples( zilch& ) ;

    /*******************************
    *   UNUSED FINDING FUNCTIONS   *
    *******************************/
    static void checkingUserInput( zilch& ) ;

    /********************************
    *   ENDGAME AND TIE FUNCTIONS   *
    ********************************/
    static void lastTurnOpportunity( zilch&, unsigned ) ;
    static void tiedEnding( zilch& ) ;

    /***************************
    *   Aesthetics Functions   *
    ***************************/
    static void pauseAndContinue ( zilch&, unsigned) ;
    static void showDice( zilch& );
    static void clear();
    static void printInstructions( zilch&, printOptions );

    /****************************
    *   GET AND SET FUNCTIONS   *
    ****************************/

    ///   Game Size ( Number of Dice ) Functions   ///
    unsigned getNumOfDiceInPlay() const
    {
        return static_cast<unsigned>(numOfDiceInPlay);
    }
    void setNumOfDiceInPlay (long numOfDice = -20 )
    {
        repeat:
        for (const auto die : diceSetMap)
            if (die.second == 0) {
                diceSetMap.erase(die.first);
                goto repeat;
            }

        if ( numOfDice > 6 )
            numOfDiceInPlay = 6;
        else if (numOfDice == -20)
        {
            numOfDiceInPlay = 0;
            for (const auto die : diceSetMap)
                numOfDiceInPlay += die.second;
        }
        else if ( numOfDice < 0 ) {
            if (abs(numOfDice) <= numOfDiceInPlay)
                numOfDiceInPlay += numOfDice;
            else numOfDiceInPlay = 6;
        }
        else if ( ( numOfDice == 0 ) || ( numOfDice == 6 ) )
        {
            setValOfChosenMultiple(0);
            if (getScoreFromMultiples() != 0) {
                setTurnScores(getScoreFromMultiples() + getScoreFromSingles());
            }
            numOfDiceInPlay = numOfDice;
        }
        else numOfDiceInPlay = numOfDice;
    }

    ///   Global Variable to keep track of current player   ///
    std::string getCurrentPlayer() const
    {
        return currentPlayer;
    }
    void setCurrentPlayer ( std::string player)
    {
        currentPlayer = std::move(player);
    }
    void incCurrentPlayer ()
    {
        for (int i = 0; i < players.size(); i++)
            if (players.at(i) == currentPlayer) {
                currentPlayer = (i == players.size() - 1) ? players.at(0) : players.at(i + 1);
                break;
            }
    }

    ///   Global Variable for knowing the multiple add-on choice   ///
    unsigned getValOfChosenMultiple() const
    {
        return valOfChosenMultiple;
    }
    void setValOfChosenMultiple ( unsigned val )
    {
        valOfChosenMultiple = val;
    }

    ///   More a debugging function than anything   ///
    unsigned getValOfAvailableMultiple() const
    {
        return valOfAvailableMultiple;
    }
    void setValOfAvailableMultiple ( unsigned val )
    {
        valOfAvailableMultiple = val;
    }

    ///   Global Variable for Accessing the Amount of Players   ///
    unsigned getAmountOfPlayers() const
    {
        return amountOfPlayers;
    }
    void setAmountOfPlayers (unsigned val )
    {
        if ( val == 0 )
            throw std::invalid_argument ( "You cannot have 0 players" );
        amountOfPlayers = val;
        players.resize ( val );
    }

    ///   Global Variable for Accessing the Amount of Players   ///
    unsigned getScoreLimit() const
    {
        return scoreLimit;
    }
    void setScoreLimit ( unsigned val )
    {
        if ( val == 0 )
        {
            std::cout << "scoreFromSingles limit defaulted to 5000" << std::endl;
            scoreLimit = 5000;
        }
        scoreLimit = val;
    }

    ///      ///
    bool getOptionSelectedBool() const
    {
        return optionSelected;
    }
    void setOptionSelectedBool(bool val)
    {
        optionSelected = val;
    }

    ///      ///
    bool getContinueSelectingBool() const
    {
        return continueSelecting;
    }
    void setContinueSelectingBool(bool val)
    {
        continueSelecting = val;
    }

    ///   scoreFromSingles Functions SCORE   ///
    unsigned getScoreFromMultiples()
    {
        return scoreFromMultiples[getCurrentPlayer()];
    }
    void setScoreFromMultiples( unsigned val )
    {
        scoreFromMultiples[getCurrentPlayer()] = ( val == 0 ) ? 0 :
                                                 ( val < 10 ) ? scoreFromMultiples[getCurrentPlayer()] * static_cast<unsigned>(pow(2,val)) : val;
    }
    unsigned getScoreFromSingles()
    {
        return scoreFromSingles[getCurrentPlayer()];
    }
    void setScoreFromSingles(unsigned val )
    {
        scoreFromSingles[getCurrentPlayer()] = (val == 0 ) ? 0 : scoreFromSingles[getCurrentPlayer()] + val;
    }

    ///   scoreFromSingles Functions   ///


    ///   Running and Turn Score   ///
    unsigned getRunningScore()
    {
        unsigned runningScore = permanentScore[getCurrentPlayer()] + scoreFromSingles[getCurrentPlayer()] + scoreFromMultiples[getCurrentPlayer()];
        return runningScore;
    }
    unsigned getTurnScore()
    {
        return turnScore[getCurrentPlayer()];
    }
    void setTurnScores(unsigned val)
    {
        turnScore[getCurrentPlayer()] = (val == 0) ? 0 : turnScore[getCurrentPlayer()] + val;
        setScoreFromSingles(0);
        setScoreFromMultiples(0);
    }

    ///   All time Permanent turnScore   ///
    unsigned getPermanentScore( const std::string& player = "" )
    {
        return permanentScore[(!player.empty() ? player : getCurrentPlayer())];
    }
    void setPermanentScore ( unsigned val, bool INIT = false )
    {
        if (!INIT) {
            if (getRunningScore() >= 1000)
                permanentScore[getCurrentPlayer()] += val;
        }
        else permanentScore[getCurrentPlayer()] = 0;
    }

    ///  Gets the Highest Score   ///
    static std::string getHighestScoringPlayer( zilch& game) {
        std::string playerWithHighestScore;
        for ( const auto & player : game.getPlayers())
            for ( const auto & player2 : game.getPlayers())
                if (game.getPermanentScore( player2 ) > game.getPermanentScore( player ) )
                    playerWithHighestScore = player2;
        return playerWithHighestScore;
    }

    ///   Global Variable containing the players   ///
    std::string getPlayer ( unsigned i )
    {
        return players.at(i);
    }
    std::vector<std::string> getPlayers()
    {
        return players;
    }
    void setPlayer ( unsigned i, std::string& player )
    {
        players.at(i) = player;
    }

    /*************************
    *   OPERATOR OVERLOADS   *
    *************************/
    friend std::istream& operator>> (std::istream&, zilch&);

    /************************
    *   PRIVATE VARIABLES   *
    ************************/
private:
    unsigned long numOfDiceInPlay = 6; // Number of dice
    std::string currentPlayer;
    unsigned valOfChosenMultiple = 0; // Possible Multiple Add-on choice
    unsigned valOfAvailableMultiple = 0;
    unsigned bust50 = 0;
    unsigned amountOfPlayers = 2;
    unsigned scoreLimit = 2000;
    bool optionSelected = false;
    bool continueSelecting = true;
    std::map<unsigned, unsigned> diceSetMap;
    std::map<std::string, unsigned> scoreFromSingles; // This is used as the general turn score when logging or the score other than that of multiples during regular turn operation
    std::map<std::string, unsigned> scoreFromMultiples;
    std::map<std::string, unsigned> turnScore;
    std::map<std::string, unsigned> permanentScore;
    std::vector<std::string> players = {""};
};

#endif // ZILCH_ZILCH_H
