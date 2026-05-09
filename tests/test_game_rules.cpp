#include "Checker.h"
#include "GameManager.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {
void expect(const bool condition, const std::string& message)
{
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

bool hasOption(const std::vector<Checker::Option>& options, const std::string& label)
{
    for (const Checker::Option& option : options)
        if (option.label == label)
            return true;
    return false;
}

void runOption(const std::vector<Checker::Option>& options, const std::string& label)
{
    for (const Checker::Option& option : options) {
        if (option.label == label) {
            option.action();
            return;
        }
    }
    expect(false, "missing option: " + label);
}

void prepareTurn(GameManager& game, const std::vector<std::string>& players)
{
    game.setPlayers(players);
    game.setScoreLimit(5000);
    game.startTurn(0);
}

void singlesDoNotMasqueradeAsMultiples()
{
    GameManager game;
    prepareTurn(game, {"A", "B"});

    Dice& dice = game.getCurrentPlayer()->getDice();
    dice.diceSetMap = {{1, 3}, {5, 1}};
    dice.setNumDiceInPlay(4);
    dice.calculateMultipleAvailability();

    const Checker checker(game);
    const std::vector<Checker::Option> options = checker.getAvailableOptions();

    expect(hasOption(options, "Score multiples of 1"), "triple ones should offer a multiple option");
    expect(!hasOption(options, "Score single 1 (100)"), "triple ones should not expose a misleading single-one option");
    expect(hasOption(options, "Score single 5 (50)"), "standalone five should remain scoreable");

    runOption(options, "Score single 5 (50)");
    expect(game.getCurrentPlayer()->getScore().getRoundScore() == 50, "single five should score exactly 50");
    expect(dice.getNumDiceInPlay() == 3, "single five should remove one die");
    expect(dice.diceSetMap.count(5) == 0, "single five should remove only the five");
    expect(dice.diceSetMap.at(1) == 3, "single five should leave the triple ones intact");
}

void singlesScoreOnlyOneDie()
{
    GameManager game;
    prepareTurn(game, {"A", "B"});

    Dice& dice = game.getCurrentPlayer()->getDice();
    dice.diceSetMap = {{1, 2}};
    dice.setNumDiceInPlay(2);
    dice.calculateMultipleAvailability();

    const Checker checker(game);
    const std::vector<Checker::Option> options = checker.getAvailableOptions();
    runOption(options, "Score single 1 (100)");

    expect(game.getCurrentPlayer()->getScore().getRoundScore() == 100, "single one should score exactly 100");
    expect(dice.getNumDiceInPlay() == 1, "single one should remove exactly one die");
    expect(dice.diceSetMap.at(1) == 1, "one die of value one should remain");
}

void finalRoundRequiresBeatingLeaderWhenTiesDisabled()
{
    GameManager game;
    game.setPlayers({"A", "B", "C"});
    game.setScoreLimit(5000);
    game.accessRuleConfig().setAllowTies(false);

    game.accessPlayers()[1].getScore().increasePermanentScore(5000);
    game.startTurn(1);
    game.startFinalRound();
    expect(game.getFinalRoundLeaderIndex() == 1, "final-round starter should start as leader");

    game.accessPlayers()[0].getScore().increasePermanentScore(5000);
    game.startTurn(0);
    game.updateFinalRoundLeaderForCurrentPlayer();
    expect(game.getFinalRoundLeaderIndex() == 1, "tying the leader should not replace them when ties are disabled");

    game.accessPlayers()[2].getScore().increasePermanentScore(5100);
    game.startTurn(2);
    game.updateFinalRoundLeaderForCurrentPlayer();
    expect(game.getFinalRoundLeaderIndex() == 2, "beating the leader should replace them");
}

void emptyPlayerResetIsSafe()
{
    GameManager game;
    prepareTurn(game, {"A", "B"});

    game.setPlayers({});
    expect(game.getPlayers().empty(), "empty player reset should clear players");
    expect(game.getCurrentPlayer() == nullptr, "empty player reset should clear current player");

    game.manageDiceCount(3);
    expect(game.getCurrentPlayer() == nullptr, "managing dice without players should remain safe");
}
}

int main()
{
    singlesDoNotMasqueradeAsMultiples();
    singlesScoreOnlyOneDie();
    finalRoundRequiresBeatingLeaderWhenTiesDisabled();
    emptyPlayerResetIsSafe();

    std::cout << "All rule tests passed.\n";
    return EXIT_SUCCESS;
}
