// Score.h
/**
Author: Jacob Anderson
Class: ECE-6122-A
Last Date Modified: 11/14/2023

Description:
Manages player scoring, including permanent and round scores, score limits, and scoring from multiples.
*/

#ifndef ZILCH_SCORE_H
#define ZILCH_SCORE_H

#include <cstdint>
#include <string>

/**
 * @class Score Score.h "Score.h"
 *
 * @brief Manages player scoring, including permanent and round scores, score limits, and scoring from multiples.
 */
class Score {
public:
    // Constructor initializes scores and score limit.
    Score() : scoreLimit(5000), permanentScore(0), roundScore(0), scoreFromMultiples(0) {}

    // Accessor and mutator methods for score limit and scores.
    [[nodiscard]] uint32_t getScoreLimit() const { return scoreLimit; }
    void setScoreLimit(const uint32_t limit) { scoreLimit = limit; }

    // Permanent (Banked) score methods
    [[nodiscard]] uint32_t getPermanentScore() const { return permanentScore; }
    //void setPermanentScore(uint32_t score) { permanentScore = score; }
    void increasePermanentScore(const uint32_t score) { permanentScore += score; }

    // Round score methods
    [[nodiscard]] uint32_t getRoundScore() const { return roundScore; }
    void setRoundScore(const uint32_t score) { roundScore = score; }
    void increaseRoundScore(const uint32_t score) { roundScore += score; }

    // Score from multiples methods
    [[nodiscard]] uint32_t getScoreFromMultiples() const { return scoreFromMultiples; }
    void setScoreFromMultiples(const uint32_t score) { scoreFromMultiples = score; }

    // Deleted methods to prevent string-based score display
    void displayCurrentScore(const std::string&) const = delete;
    void displayHighScoreInfo(const std::string&, const std::string&) const = delete;

private:
    uint32_t scoreLimit;
    uint32_t permanentScore;    // Banked score
    uint32_t roundScore;
    uint32_t scoreFromMultiples;
};

#endif //ZILCH_SCORE_H
