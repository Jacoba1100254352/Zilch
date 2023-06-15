#ifndef ZILCH_ZILCH_H
#define ZILCH_ZILCH_H

#include <iostream>
#include <map>
#include <vector>
#include <stdexcept>

class Zilch {
public:
    void playGame();

private:
    std::map<uint8_t, uint8_t> diceSetMap;
    std::vector<std::string> players;
    std::string currentPlayer;
    uint8_t numOfDiceInPlay;
    uint32_t turnScore;
    uint32_t runningScore;
    uint32_t scoreFromMultiples;
    uint32_t scoreFromSingles;
    uint32_t scoreLimit;
    uint8_t valOfChosenMultiple;
    uint8_t valOfAvailableMultiple;
    bool optionSelectedBool;
    bool continueTurnBool;
    bool continueSelectingBool;

    void initializeGame();
    void printGameInfo();
    void addPlayer(const std::string& playerName);
    uint32_t getPermanentScore(const std::string& playerName) const;
    std::string getHighestScoringPlayer() const;
    uint8_t getAmountOfPlayers() const;
    uint8_t getNumOfDiceInPlay() const;
    void setNumOfDiceInPlay(int8_t change = 0);
    uint32_t getTurnScore() const;
    void setTurnScores(uint32_t score);
    uint32_t getRunningScore() const;
    void setRunningScore(uint32_t score);
    uint32_t getScoreFromMultiples() const;
    void setScoreFromMultiples(uint32_t score);
    uint32_t getScoreFromSingles() const;
    void setScoreFromSingles(uint32_t score);
    uint32_t getScoreLimit() const;
    uint8_t getValOfChosenMultiple() const;
    void setValOfChosenMultiple(uint8_t val);
    uint8_t getValOfAvailableMultiple() const;
    void setValOfAvailableMultiple(uint8_t val);
    bool isOptionAvailable() const;
    bool getOptionSelectedBool() const;
    void setOptionSelectedBool(bool value);
    bool getContinueTurnBool() const;
    bool getContinueSelectingBool() const;
    void setContinueTurnBool(bool turnBool, bool selectingBool = false);
    bool isStrait() const;
    bool isSet() const;
    bool isDesiredMultipleAvailable(uint8_t val) const;
    bool canAddMultiples() const;
    bool isSingle(uint8_t val) const;

    friend std::istream& operator>>(std::istream& is, Zilch& game);
};

std::istream& operator>>(std::istream& is, Zilch& game);
void pauseAndContinue(const Zilch& game, bool showCurrentScore = false);
void showDice(const Zilch& game);
void clear();
void printInstructions(const Zilch& game, int options);



#endif //ZILCH_ZILCH_H
