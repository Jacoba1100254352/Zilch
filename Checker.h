#ifndef ZILCH_CHECKER_H
#define ZILCH_CHECKER_H

#include "Zilch.h"

class Checker {
public:
    explicit Checker(Zilch& game) : game(game) {} // Constructor initializes the member variable

    void check();
    void checkingUserInput();
    void straits();
    void set();
    void multiple(uint8_t val);
    void single(uint8_t val);
    void updateValOfAvailableMultiples();
    void applyAllPossibleOptions();
    void lastTurnOpportunity(uint8_t FULL_SET_OF_DICE = 6);
    void tiedEnding();

    [[nodiscard]] bool isStrait();
    [[nodiscard]] bool isSet();
    [[nodiscard]] bool isMultiple();
    [[nodiscard]] bool isDesiredMultipleAvailable(uint8_t val);
    [[nodiscard]] bool canAddMultiples();
    [[nodiscard]] bool isSingle(uint8_t val);

private:
    Zilch game; // Member variable holding a reference to the Zilch instance
};



#endif //ZILCH_CHECKER_H
