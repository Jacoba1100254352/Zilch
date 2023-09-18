#ifndef ZILCH_CHECKER_H
#define ZILCH_CHECKER_H

#include <cstdint>
#include <iostream>

class GameManager;

class Checker {
public:
    explicit Checker(GameManager& game) : game(game) {} // Constructor initializes the member variable

    void handleFirstRollBust();
    void handleBust();
    void printPossibleMultipleAddition();
    static void ignoreRemainingInput();
    bool enterEndTurnOption();
    void updateGameStatus(uint16_t playOrEndTurn);
    void handleNoOptionsLeft();

    void check();
    void checkUserInput();

    void checkStraits();
    void checkSet();
    void checkMultiple(uint16_t val);
    void checkSingle(uint16_t val);

    void applyPossibleOptions();

    [[nodiscard]] bool isStrait() const;
    [[nodiscard]] bool isSet() const;
    [[nodiscard]] bool isMultiple() const;
    [[nodiscard]] bool isDesiredMultipleAvailable(uint16_t val) const;
    [[nodiscard]] bool canAddMultiples() const;
    [[nodiscard]] bool isSingle(uint16_t val) const;

    // OptionAvailability
    [[nodiscard]] bool isOptionAvailable() const;

    void readInput(std::istream& input);

private:
    GameManager& game; // Member variable holding a reference to the GameManager instance

    // Input Functions
    void executeSelection(uint16_t val);
    void executeSecondaryCommand(std::istream& input);
    bool canProcessMultiple(uint16_t val);
    void handleFailedMultiple(uint16_t val);
    void processZeroCommand();
    void displayPossibleOptions();
    static void displayImpossibleOptionMessage();
};



#endif //ZILCH_CHECKER_H
