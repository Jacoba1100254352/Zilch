#ifndef ZILCH_ZILCH_H
#define ZILCH_ZILCH_H

#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

class zilch
{
public:
    /******************
    *   CONSTRUCTOR   *
    ******************/
    //explicit zilch (unsigned ndip = 6, unsigned cpp = 0, unsigned npm = 0, unsigned vcm = 0, unsigned vam = 0, unsigned b50 = 0,
    //                unsigned ap = 0, unsigned dsv = 0, unsigned s = 0, unsigned ps = 0, unsigned sl = 2000 );

    /************************
    *   ROLLING FUNCTIONS   *
    ************************/
    static unsigned roll() ;
    static void rollSixDice( zilch& ) ;
    static void showDice(const zilch& );

    /*******************************
    *   DUMMY CHECKING FUNCTIONS   *
    *******************************/
    static bool straitBool( const zilch& ) ;
    static bool setBool( const zilch& ) ;
    static bool desiredMultipleAvailabilityBool ( const zilch&, unsigned ) ; // Previously mValPass
    static bool multiplesBool( const zilch& ) ;
    static bool multiplesAddBool( const zilch& ) ;
    static bool singleBool( const zilch& ) ;
    static bool bust50Bool( const zilch& ) ;
    static bool winBool( const zilch& ) ;

    /****************************************
    *   CHECKING AND OUTPUTTING FUNCTIONS   *
    ****************************************/
    static void straits( zilch& ) ;
    static void set( zilch& ) ;
    static void multiple ( zilch&, unsigned ) ;
    static void multipleAdd ( zilch&, unsigned ) ;
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
    static void tiedEnding( zilch&, unsigned, unsigned ) ;

    /***************************
    *   Aesthetics Functions   *
    ***************************/
    static void pauseAndContinue ( zilch&, unsigned ) ;

    /****************************
    *   GET AND SET FUNCTIONS   *
    ****************************/

    ///   Game Size ( Number of Dice ) Functions GAMESIZE   ///
    unsigned getNumOfDiceInPlay() const
    {
        return static_cast<unsigned>(numOfDiceInPlay);
    }
    void setNumOfDiceInPlay (const unsigned long numOfDice )
    {
        if ( numOfDice > 6 )
            numOfDiceInPlay = 6;
        else if ( numOfDice == 0 || numOfDice == 6 )
        {
            setNumOfPrevMultiples(0);
            setValOfChosenMultiple(0);
            numOfDiceInPlay = 6;
        }
        else numOfDiceInPlay = numOfDice;
    }

    ///   Global Variable for Looping Through Players KPASS   ///
    unsigned getCurrentPlayerPosition() const
    {
        return currentPlayerPosition;
    }
    void setCurrentPlayerPosition (const unsigned val )
    {
        if ( val == 0 )
            currentPlayerPosition = 0;
        else
        {
            currentPlayerPosition += val;
            currentPlayerPosition %= amountOfPlayers;
        }
    }

    ///   Global Variable for Checking for Previous Multiples MPASS   ///
    unsigned getNumOfPrevMultiples() const
    {
        return numOfPrevMultiples;
    }
    void setNumOfPrevMultiples (const unsigned val ) // if there was a multiple chosen numOfPrevMultiples = 1
    {
        if ( val == 0 )
            numOfPrevMultiples = 0;
        else
            numOfPrevMultiples += 1;
    }

    ///   Global Variable for knowing the multiple add-on choice MVALPASS   ///
    unsigned getValOfChosenMultiple() const
    {
        return valOfChosenMultiple;
    }
    void setValOfChosenMultiple (const unsigned val )
    {
        valOfChosenMultiple = val;
    }

    ///   More a debugging function than anything   ///
    unsigned getValOfAvailableMultiple() const
    {
        return valOfAvailableMultiple;
    }
    void setValOfAvailableMultiple (const unsigned val )
    {
        valOfAvailableMultiple = val;
    }

    ///   Making sure you can only bust on your first roll once BUST50   ///
/*    unsigned GetBust50() const
    {
        return bust50;
    }
    void SetBust50 ( unsigned val )
    {
        bust50 = val;
    }*/

    ///   Global Variable for Accessing the Amount of Players PLAYERSIZEPASS   ///
    unsigned getAmountOfPlayers() const
    {
        return amountOfPlayers;
    }
    void setAmountOfPlayers (const unsigned val )
    {
        if ( val == 0 )
            throw std::invalid_argument ( "You cannot have 0 players" );
        amountOfPlayers = val;
        turnScore.resize ( val );
        permanentScore.resize ( val );
        players.resize ( val );
    }

    ///   Global Variable for Accessing the Amount of Players PLAYERSIZEPASS   ///
    unsigned getScoreLimit() const
    {
        return scoreLimit;
    }
    void setScoreLimit (const unsigned val )
    {
        if ( val == 0 )
        {
            std::cout << "turnScore limit defaulted to 5000" << std::endl;
            scoreLimit = 5000;
        }
        scoreLimit = val;
    }

/*    unsigned getDiceSetVector() const
    {
        return turnScore.at(getCurrentPlayerPosition());
    }
    void setDiceSetVector()
    {
        diceSetVector = { roll(), roll(), roll(), roll(), roll(), roll() };
    }*/

    ///   turnScore Functions SCORE   ///
    unsigned getRunningScore() const
    {
        unsigned runningScore = permanentScore.at(getCurrentPlayerPosition()) + turnScore.at(getCurrentPlayerPosition());
        return runningScore;
    }
    unsigned getTurnScore() const
    {
        return turnScore.at(getCurrentPlayerPosition());
    }
    void setTurnScore(const unsigned val )
    {
        if ( val == 0 )
            turnScore.at(getCurrentPlayerPosition()) = 0;
        else
            turnScore.at(getCurrentPlayerPosition()) += val;
    }

    ///   All time Permanent turnScore PERMANENTSCORE   ///
    unsigned getPermanentScore(const unsigned i = 0 ) const
    {
        if ( i != 0 )
            return permanentScore.at(i);
        return permanentScore.at(getCurrentPlayerPosition());
    }
    void setPermanentScore (const unsigned val )
    {
        if ((getRunningScore() >= 1000 ) || (val == 0 ) )
        {
            if ( val == 0 )
                permanentScore.at(getCurrentPlayerPosition()) = 0;
            else
                permanentScore.at(getCurrentPlayerPosition()) += val;
        }
    }

    ///   Global Variable containing the players PLAYERS   ///
    std::string getPlayers()
    {
        return players.at(getCurrentPlayerPosition());
    }
    std::string getPlayers (const unsigned i )
    {
        return players.at(i);
    }
    void setPlayers (const unsigned i, const std::string& player )
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
    unsigned currentPlayerPosition = 0; // loops through/keeps track of current player throughout program
    unsigned numOfPrevMultiples = 0; // Contains number of multiples chosen (0-2)
    unsigned valOfChosenMultiple = 0; // Possible Multiple Add-on choice
    unsigned valOfAvailableMultiple = 0;
    unsigned bust50 = 0;
    unsigned amountOfPlayers = 2;
    unsigned scoreLimit = 2000;
    std::vector<unsigned> diceSetVector = {0};
    std::vector<unsigned> turnScore = {0};
    std::vector<unsigned> permanentScore = {0};
    std::vector<std::string> players = {""};
};

#endif // ZILCH_ZILCH_H
