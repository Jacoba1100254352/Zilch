#ifndef ZILCH_CHECKER_H
#define ZILCH_CHECKER_H

#include <cstdint>
#include <iostream>

class GameManager;

class Checker {
public:
    explicit Checker(GameManager&game) : game(game) {} // Constructor initializes the member variable

    void handleFirstRollBust() const;

    void handleBust() const;

    void printPossibleMultipleAddition() const;

    static void ignoreRemainingInput();

    [[nodiscard]] bool enterEndTurnOption() const;

    void updateGameStatus(uint16_t playOrEndTurn) const;

    void handleNoOptionsLeft() const;

    void check() const;

    void checkUserInput() const;

    void checkStraits() const;

    void checkSet() const;

    void checkMultiple(uint16_t dieValue) const;

    void checkSingle(uint16_t dieValue) const;

    void applyPossibleOptions() const;

    [[nodiscard]] bool isStrait() const;

    [[nodiscard]] bool isSet() const;

    [[nodiscard]] bool isMultiple() const;

    [[nodiscard]] bool isDesiredMultipleAvailable(uint16_t desiredMultiple) const;

    [[nodiscard]] bool canAddMultiples() const;

    [[nodiscard]] bool isSingle(uint16_t single) const;

    // OptionAvailability
    [[nodiscard]] bool isOptionAvailable() const;

    void readInput(std::istream&input) const;

private:
    GameManager&game; // Member variable holding a reference to the GameManager instance

    // Input Functions
    void executeSelection(uint16_t val) const;

    void executeSecondaryCommand(std::istream&input) const;

    [[nodiscard]] bool canProcessMultiple(uint16_t val) const;

    void handleFailedMultiple(uint16_t chosenMultiple) const;

    void processZeroCommand() const;

    void displayPossibleOptions() const;

    static void displayImpossibleOptionMessage();
};


#endif //ZILCH_CHECKER_H
