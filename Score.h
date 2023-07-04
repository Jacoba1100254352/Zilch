#ifndef ZILCH_SCORE_H
#define ZILCH_SCORE_H

#include <cstdint>
#include <iostream>

class Score {
public:
    Score() : permanentScore(0), turnScore(0), runningScore(0), scoreFromMultiples(0), scoreFromSingles(0), scoreLimit(2000) {}

    [[nodiscard]] uint32_t getPermanentScore() const { return permanentScore; }
    //void setPermanentScore(uint32_t score) { permanentScore = score; }
    void increasePermanentScore(uint32_t score) { permanentScore += score; }

    [[nodiscard]] uint32_t getTurnScore() const { return turnScore; }
    void setTurnScore(uint32_t score) { turnScore = score; }

    [[nodiscard]] uint32_t getRunningScore() const { return runningScore; }
    //void setRunningScore(uint32_t score) { runningScore = score; }
    void increaseRunningScore(uint32_t score) { runningScore += score; }

    [[nodiscard]] uint32_t getScoreFromMultiples() const { return scoreFromMultiples; }
    void setScoreFromMultiples(uint32_t score) { scoreFromMultiples = score; }

    [[nodiscard]] uint32_t getScoreFromSingles() const { return scoreFromSingles; }
    void setScoreFromSingles(uint32_t score) { scoreFromSingles = score; }

    [[nodiscard]] uint32_t getScoreLimit() const { return scoreLimit; }
    void setScoreLimit(uint32_t limit) {
        scoreLimit = limit;
    }

    void displayCurrentScore(const std::string& playerName) const {std::cout << playerName << "'s current score: " << runningScore << std::endl;}
    void displayHighScoreInfo(const std::string& currentPlayerName, const std::string& highestScoringPlayerName) const {
        uint32_t highestScore = permanentScore;

        ///   if: High permanent score below limit: Ask to end or keep playing   ///
        if (highestScore < scoreLimit)
            displayCurrentScore(currentPlayerName);
            ///   else if: Someone has surpassed the limit: try to beat them   ///
        else if (highestScore > runningScore)
            std::cout << "\n\nYour current score of " << runningScore << " is " << highestScore - runningScore
                      << " less than " << currentPlayerName << "'s High Score of " << highestScore << " so keep going! :)"
                      << std::endl;
            ///   else if: Tied for the highest   ///
        else if (highestScoringPlayerName != currentPlayerName)
            std::cout << "You are currently tied with the highest scoring player!";
            ///   else: You are the highest! Ask to end or keep playing   ///
        else std::cout << "You are currently the highest scoring player";
    }

private:
    uint32_t permanentScore;
    uint32_t turnScore;
    uint32_t runningScore;
    uint32_t scoreFromMultiples;
    uint32_t scoreFromSingles;
    uint32_t scoreLimit;
};

#endif //ZILCH_SCORE_H
