#ifndef ZILCH_SCORE_H
#define ZILCH_SCORE_H

#include <cstdint>

struct Score {
    uint32_t permanentScore;
    uint32_t turnScore;
    uint32_t runningScore;
    uint32_t scoreFromMultiples;
    uint32_t scoreFromSingles;
    uint32_t scoreLimit;
};

#endif //ZILCH_SCORE_H
