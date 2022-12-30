#ifndef ZILCH_ZILCH_H
#define ZILCH_ZILCH_H

#include <iostream>
#include <algorithm>
#include <set>
#include <stdexcept>
#include <cstdlib>
#include <cstdint>
#include <ctime>
#include <random>
#include <cmath>
#include <map>
#include <utility>
#include <vector>
#include <climits>
#include <cstring>
#include <unistd.h>
#include <optional>

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
//#include <term.h>

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

class zilch {
public:

	/************************
	*   ENUM FOR PRINTING   *
	************************/
	enum printOptions {
		ENTER, NEXT, REENTER
	};

	/************************
	*   ROLLING FUNCTIONS   *
	************************/
	static uint8_t roll();
	static void rollSixDice(zilch&);

	/*********************************
	*   DUMMY CHECKING FUNCTIONS     *
	*********************************/
	bool isStrait() {
		return (diceSetMap.size() == 6);
	}
	bool isSet() {
		return (diceSetMap.size() == 3 && !isStrait() && !isMultiple() && !std::any_of(diceSetMap.begin(), diceSetMap.end(), [](const auto& die) { return die.second != 2; }));
	}
	///   Checks to verify that the Multiple the User wants is available   ///
	bool isDesiredMultipleAvailable(uint8_t desiredMultiple) {
		return (std::any_of(diceSetMap.begin(), diceSetMap.end(), [desiredMultiple](const auto& die) {
			return die.first == desiredMultiple && die.second >= 3;
		}));
	}
	///   Checks to verify that Multiples exist   ///
	bool isMultiple() { // or is/areMultiples
		return (std::any_of(diceSetMap.begin(), diceSetMap.end(), [](const auto& die) { return die.second >= 3; }));
	}
	bool canAddMultiples() {
		return (std::any_of(diceSetMap.begin(), diceSetMap.end(), [this](const auto& die) { return die.first == getValOfChosenMultiple(); }));
	}
	bool isSingle(uint8_t single) {
		return (std::any_of(diceSetMap.begin(), diceSetMap.end(), [single](const auto& die) { return die.first == single; }));
	}
	bool isWin() {
		// adjust and maybe make this an std::any_of function
		for (uint8_t i = 0; i < getAmountOfPlayers(); i++)
			if (permanentScore[getCurrentPlayer()] >= getScoreLimit())
				return true;
		return false;
	}
	bool isOptionAvailable() {
		return isStrait() || isSet() || isMultiple() || isSingle(1) || zilch::isSingle(5) || zilch::canAddMultiples();
	}

	/****************************************
	*   CHECKING AND OUTPUTTING FUNCTIONS   *
	****************************************/
	static void straits(zilch&);
	static void set(zilch&);
	static void multiple(zilch&, uint8_t);
	static void single(zilch&, uint8_t);
	static void check(zilch&);
	static void updateValOfAvailableMultiples(zilch&);
	static void applyAllPossibleOptions(zilch&);

	/*******************************
	*   UNUSED FINDING FUNCTIONS   *
	*******************************/
	static void checkingUserInput(zilch&);

	/********************************
	*   ENDGAME AND TIE FUNCTIONS   *
	********************************/
	static void lastTurnOpportunity(zilch&, int8_t);
	static void tiedEnding(zilch&);

	/***************************
	*   Aesthetics Functions   *
	***************************/
	static void pauseAndContinue(zilch&, bool = false);
	static void showDice(zilch&);
	static void clear();
	static void printInstructions(zilch&, printOptions);

	/****************************
	*   GET AND SET FUNCTIONS   *
	****************************/

	///   Game Size ( Number of Dice ) Functions   ///
	[[nodiscard]] uint8_t getNumOfDiceInPlay() const {
		return static_cast<uint8_t>(numOfDiceInPlay);
	}
	void setNumOfDiceInPlay(int8_t numOfDice = -20) {
		bool repeat;
		do {
			repeat = false;
			for (const auto die : diceSetMap)
				if (die.second == 0) {
					diceSetMap.erase(die.first);
					repeat = true;
					break;
				}
		} while (repeat);


		if (numOfDice > 6)
			numOfDiceInPlay = 6;
		else if (numOfDice == -20) // Recount the number of dice available
		{
			numOfDiceInPlay = 0;
			for (const auto die : diceSetMap)
				numOfDiceInPlay += die.second;
		} else if (numOfDice < 0) { // (-1) Subtract or reset dice
			numOfDiceInPlay = (abs(numOfDice) <= numOfDiceInPlay) ? numOfDiceInPlay + numOfDice : 6;
		} else if ((numOfDice == 0) || (numOfDice == 6)) // New roll or new turn
		{
			setValOfChosenMultiple(0);

			setTurnScores(getScoreFromMultiples() + getScoreFromSingles());
			numOfDiceInPlay = numOfDice;
		} else numOfDiceInPlay = numOfDice;

		if (numOfDice == 6)
			continueTurn = true; // Using the set function creates a recursive call chain
	}

	///   Global Variable to keep track of current player   ///
	[[nodiscard]] std::string getCurrentPlayer() const {
		return currentPlayer;
	}
	void setCurrentPlayer(std::string player) {
		currentPlayer = std::move(player);
	}
	void incCurrentPlayer() {
		for (int i = 0; i < players.size(); i++)
			if (players.at(i) == currentPlayer) {
				currentPlayer = (i == players.size() - 1) ? players.at(0) : players.at(i + 1);
				break;
			}
		setContinueTurnBool(true);
	}

	///   Initialize Map Sizes/Resize   ///
	void initializeMaps() {
		scoreFromSingles.clear();
		scoreFromMultiples.clear();
		turnScore.clear();
		permanentScore.clear();
		for (const std::string& player : players) {
			scoreFromSingles[player] = 0;
			scoreFromMultiples[player] = 0;
			turnScore[player] = 0;
			permanentScore[player] = 0;
		}

		if (scoreFromSingles.size() != amountOfPlayers || scoreFromMultiples.size() != amountOfPlayers || turnScore.size() != amountOfPlayers || permanentScore.size() != amountOfPlayers)
			throw std::length_error("Score maps are incorrectly sized, File: " + std::string(__FILE_NAME__) + ", Line: " + std::to_string(__LINE__));
	}

	///   Global Variable for knowing the multiple add-on choice   ///
	[[nodiscard]] uint8_t getValOfChosenMultiple() const {
		return valOfChosenMultiple;
	}
	void setValOfChosenMultiple(uint8_t val) {
		valOfChosenMultiple = val;
	}

	///   More a debugging function than anything   ///
	[[nodiscard]] uint8_t getValOfAvailableMultiple() const {
		return valOfAvailableMultiple;
	}
	void setValOfAvailableMultiple(uint8_t val) {
		valOfAvailableMultiple = val;
	}

	///   Global Variable for Accessing the Amount of Players   ///
	[[nodiscard]] uint8_t getAmountOfPlayers() const {
		// adjust and make simplify by getting rid of this variable and returning the size of players,
		// maybe use a class/constructor to handle and initialize players
		return amountOfPlayers;
	}
	void setAmountOfPlayers(uint8_t val) {
		if (!val)
			throw std::invalid_argument("You cannot have 0 players");
		amountOfPlayers = val;
		players.resize(val);
	}

	///   Global Variable for Accessing the Amount of Players   ///
	[[nodiscard]] uint32_t getScoreLimit() const {
		return scoreLimit;
	}
	void setScoreLimit(uint32_t val) {
		if (!val) { // val == 0
			std::cout << "scoreFromSingles limit defaulted to 5000" << std::endl;
			scoreLimit = 5000;
		}
		scoreLimit = val;
	}

	///      ///
	[[nodiscard]] bool getOptionSelectedBool() const {
		return optionSelected;
	}
	void setOptionSelectedBool(bool val) {
		optionSelected = val;
	}

	///      ///
	[[nodiscard]] bool getContinueSelectingBool() const {
		return continueSelecting;
	}
	void setContinueSelectingBool(bool val) {
		if (!val && !getOptionSelectedBool()) {
			std::cout << "You must select at least one option" << std::endl;
			val = true;
		}
		continueSelecting = val;
	}

	///      ///
	[[nodiscard]] bool getContinueTurnBool() const {
		return continueTurn;
	}
	void setContinueTurnBool(bool val, bool bust = false) {
		if (!val && !bust) {
			///   Greater than or equal to 1000: log and end turn   ///
			if (getRunningScore() >= 1000) {

				///   Update Scores   ///
				setTurnScores(getScoreFromSingles() + getScoreFromMultiples());

				///   Log and end turn   ///
				setPermanentScore(getTurnScore());
				std::cout << std::endl << getCurrentPlayer() << "'s permanent score has been logged and is now: " << getPermanentScore() << std::endl << std::endl;
				setNumOfDiceInPlay(0);
			}

				///   else if: No option selected and not above 1000: select an option   ///
			else if (!getOptionSelectedBool()) {
				std::cout << "You are not allowed to end without a permanent or running score higher than 1000" << std::endl;
				val = true;
				//zilch::pauseAndContinue(game, 1 );
				//printInstructions(game, REENTER);
			} /// else continueSelecting = false, roll again
		} else if (val && bust) val = false;
		continueTurn = val;
	}

	///   scoreFromSingles Functions   ///
    uint16_t getScoreFromSingles() {
		return scoreFromSingles[getCurrentPlayer()];
	}
	void setScoreFromSingles(uint16_t val) {
		scoreFromSingles[getCurrentPlayer()] = (!val) ? 0 : scoreFromSingles[getCurrentPlayer()] + val;
	}

	///   scoreFromMultiples Functions   ///
	uint32_t getScoreFromMultiples() {
		return scoreFromMultiples[getCurrentPlayer()];
	}
	void setScoreFromMultiples(uint16_t val) {
		scoreFromMultiples[getCurrentPlayer()] = (!val) ? 0 : (val < 10) ? scoreFromMultiples[getCurrentPlayer()] * static_cast<uint16_t>(pow(2, val)) : val;
	}

	///   Running and Turn Score   ///
    uint32_t getRunningScore() {
        uint32_t runningScore = permanentScore[getCurrentPlayer()] + turnScore[getCurrentPlayer()] + scoreFromSingles[getCurrentPlayer()] + scoreFromMultiples[getCurrentPlayer()];
		return runningScore;
	}
    uint32_t getTurnScore() {
		return turnScore[getCurrentPlayer()];
	}
	void setTurnScores(uint32_t val) {
		turnScore[getCurrentPlayer()] = (!val || (!scoreFromMultiples[getCurrentPlayer()] && !scoreFromSingles[getCurrentPlayer()])) ? 0 : turnScore[getCurrentPlayer()] + val;
		setScoreFromSingles(0);
		setScoreFromMultiples(0);
	}

	///   All time Permanent turnScore   ///
    uint32_t getPermanentScore(const std::string& player = "") {
		return permanentScore[(!player.empty() ? player : getCurrentPlayer())];
	}
	void setPermanentScore(uint32_t val, bool INIT = false) {
		if (!INIT) {
			if (getRunningScore() >= 1000)
				permanentScore[getCurrentPlayer()] += val;
		} else permanentScore[getCurrentPlayer()] = 0;
	}

	///  Gets the Highest Score   ///
	std::string getHighestScoringPlayer() {
		std::string playerWithHighestScore;
		for (const std::string& player : getPlayers())
			for (const std::string& player2 : getPlayers())
				if (getPermanentScore(player2) > getPermanentScore(player))
					playerWithHighestScore = player2;
		return playerWithHighestScore;
	}

	///   Global Variable containing the players   ///
	std::string getPlayer(uint8_t i) {
		return players.at(i);
	}
	std::vector<std::string> getPlayers() {
		return players;
	}
	void setPlayer(uint8_t i, std::string& player) {
		players.at(i) = player;
	}

	/*************************
	*   OPERATOR OVERLOADS   *
	*************************/
	friend std::istream& operator>>(std::istream&, zilch&);

	/************************
	*   PRIVATE VARIABLES   *
	************************/
private:
	uint8_t numOfDiceInPlay = 6; // Number of dice
	std::string currentPlayer;
	uint8_t valOfChosenMultiple = 0; // Possible Multiple Add-on choice
	uint8_t valOfAvailableMultiple = 0;
    uint8_t amountOfPlayers = 2;
    uint32_t scoreLimit = 2000;
	bool optionSelected = false;
	bool continueSelecting = true;
	bool continueTurn = true;
	std::map<uint8_t, uint8_t> diceSetMap;
	std::map<std::string, uint16_t> scoreFromSingles; // This is used as the general turn score when logging or the score other than that of multiples during regular turn operation
	std::map<std::string, uint32_t> scoreFromMultiples;
	std::map<std::string, uint32_t> turnScore;
	std::map<std::string, uint32_t> permanentScore;
	std::vector<std::string> players = {""};
};

#endif // ZILCH_ZILCH_H
