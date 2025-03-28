// Checker.h

#ifndef CHECKER_H
#define CHECKER_H

#include <cstdint>
#include <iostream>

class GameManager;  // Forward declare to avoid circular dependency.

class Checker {
public:
    explicit Checker(GameManager& game) : game(game) {}

    // Evaluate possible scoring options, handle bust, etc.
    [[nodiscard]] bool hasValidOption() const;
    [[nodiscard]] bool isStrait() const;
    [[nodiscard]] bool isSet() const;
    [[nodiscard]] bool isMultipleAvailable() const;
    [[nodiscard]] bool isSingleAvailable() const;

    void handleBust(bool isFirstRoll) const;
    void handleStrait() const;
    void handleSet() const;
    void handleMultiple(uint16_t dieValue) const;
    void handleSingle(uint16_t dieValue) const;

    void getCurrentPlayer() const;

private:
    GameManager& game;
};

#endif // CHECKER_H
