#ifndef ZILCH_SCORE_H
#define ZILCH_SCORE_H

#include <cstdint>
#include <iostream>

class Score {
public:
    Score() : scoreLimit(2000), permanentScore(0), roundScore(0), scoreFromMultiples(0) {}

    [[nodiscard]] uint32_t getScoreLimit() const { return scoreLimit; }
    void setScoreLimit(const uint32_t limit) { scoreLimit = limit; }

    [[nodiscard]] uint32_t getPermanentScore() const { return permanentScore; }
    //void setPermanentScore(uint32_t score) { permanentScore = score; }
    void increasePermanentScore(const uint32_t score) { permanentScore += score; }

    [[nodiscard]] uint32_t getRoundScore() const { return roundScore; }
    void setRoundScore(const uint32_t score) { roundScore = score; }
    void increaseRoundScore(const uint32_t score) { roundScore += score; }

    [[nodiscard]] uint32_t getScoreFromMultiples() const { return scoreFromMultiples; }
    void setScoreFromMultiples(const uint32_t score) { scoreFromMultiples = score; }

    void displayCurrentScore(const std::string& playerName) const { std::cout << playerName << "'s current score: " << roundScore << std::endl; }
    void displayHighScoreInfo(const std::string& currentPlayerName, const std::string& highestScoringPlayerName) const {
        ///   if: High permanent score below limit: Ask to end or keep playing   ///
        if (const uint32_t highestScore = permanentScore; highestScore < scoreLimit)
            displayCurrentScore(currentPlayerName);
            ///   else if: Someone has surpassed the limit: try to beat them   ///
        else if (highestScore > roundScore)
            std::cout << "\n\nYour current score of " << roundScore << " is " << highestScore - roundScore
                      << " less than " << currentPlayerName << "'s High Score of " << highestScore << " so keep going! :)"
                      << std::endl;
            ///   else if: Tied for the highest   ///
        else if (highestScoringPlayerName != currentPlayerName)
            std::cout << "You are currently tied with the highest scoring player!";
            ///   else: You are the highest! Ask to end or keep playing   ///
        else std::cout << "You are currently the highest scoring player";
    }

private:
    uint32_t scoreLimit;
    uint32_t permanentScore;
    uint32_t roundScore;
    uint32_t scoreFromMultiples;
};

#endif //ZILCH_SCORE_H
