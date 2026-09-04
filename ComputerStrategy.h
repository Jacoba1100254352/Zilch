#ifndef ZILCH_COMPUTER_STRATEGY_H
#define ZILCH_COMPUTER_STRATEGY_H

#include "Player.h"
#include "rules/GameOption.h"

#include <array>
#include <cstddef>
#include <string_view>
#include <vector>

class GameManager;

namespace zilch::computer {

enum class PostSelectionDecision {
    SelectAgain,
    Roll,
    Bank
};

enum class TurnAction {
    None,
    AcknowledgeBust,
    AcceptStealAndRoll,
    DeclineStealAndRoll,
    SelectOption,
    Roll,
    Bank
};

struct Policy {
    std::string_view name;
    std::array<int, 7> bankThresholdByDice;
    double scoreWeight;
    double remainingDiceWeight;
    double hotDiceWeight;
    double multipleWeight;
    double leadFactor;
    double trailFactor;
    double closingFactor;
    double rollBias;
};

[[nodiscard]] std::string_view difficultyName(ComputerDifficulty difficulty);
[[nodiscard]] const Policy& policyForDifficulty(ComputerDifficulty difficulty, bool stealingEnabled);
[[nodiscard]] std::size_t chooseOption(
    const GameManager& game,
    const std::vector<rules::GameOption>& options
);
[[nodiscard]] PostSelectionDecision decideAfterSelection(
    const GameManager& game,
    const std::vector<rules::GameOption>& remainingOptions
);
[[nodiscard]] TurnAction chooseTurnAction(
    const GameManager& game,
    bool waitingForRoll,
    bool awaitingSelection,
    const std::vector<rules::GameOption>& remainingOptions
);
[[nodiscard]] bool shouldBank(const GameManager& game);
[[nodiscard]] bool shouldAcceptSteal(const GameManager& game);

}

#endif // ZILCH_COMPUTER_STRATEGY_H
