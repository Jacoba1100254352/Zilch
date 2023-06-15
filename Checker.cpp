#include "Checker.h"
#include <iostream>
#include <cmath>
#include <limits>

void Checker::check(zilch& game) {
    if (!game.isOptionAvailable()) {
        if (game.getNumOfDiceInPlay() == 6 && !game.getTurnScore()) {
            zilch::showDice(game);
            std::cout << "\nYou have busted on the first roll, try again" << std::endl;
            game.setScoreFromSingles(50);
            zilch::pauseAndContinue(game, true);
        } else if (game.getNumOfDiceInPlay() >= 1) {
            zilch::showDice(game);
            std::cout << "\nYou have busted" << std::endl;
            game.setNumOfDiceInPlay(0);
            game.setTurnScores(0);
            game.setContinueTurnBool(false, true);
            zilch::pauseAndContinue(game);
        } else {
            std::cout << "\nYou have a full set of dice now" << std::endl;
            game.setNumOfDiceInPlay(6);
            zilch::pauseAndContinue(game, true);
        }
    } else {
        Checker::checkingUserInput(game);
    }
}

void Checker::checkingUserInput(zilch& game) {
    uint8_t playOrEndTurn = 0;

    do {
        zilch::clear();
        updateValOfAvailableMultiples(game);

        if (game.canAddMultiples() && (game.getScoreFromMultiples() >= 200))
            std::cout << "You can add to your multiple of " << game.getValOfChosenMultiple() << "!\n";

        if (!game.getOptionSelectedBool())
            zilch::printInstructions(game, zilch::ENTER);
        else if (game.isOptionAvailable())
            zilch::printInstructions(game, zilch::NEXT);
        else
            break;

        std::cin.clear();
        std::cin.ignore(std::numeric_limits<int>::max(), '\n');
        std::cin >> game;

        if (game.getRunningScore() >= 1000) {
            uint32_t highestScore = game.getPermanentScore(game.getHighestScoringPlayer());

            if (highestScore < game.getScoreLimit())
                std::cout << "\n" << game.getCurrentPlayer() << "'s current score: " << game.getRunningScore();
            else if (highestScore > game.getRunningScore())
                std::cout << "\n\nYour current score of " << game.getRunningScore() << " is " << highestScore - game.getRunningScore() << " less than " << game.getHighestScoringPlayer() << "'s High Score of " << highestScore << " so keep going! :)" << std::endl;
            else if (game.getHighestScoringPlayer() != game.getCurrentPlayer())
                std::cout << "You are currently tied with the highest scoring player!";
            else
                std::cout << "You are currently the highest scoring player";

            if (highestScore < game.getScoreLimit() || (highestScore >= game.getScoreLimit() && game.getRunningScore() >= highestScore)) {
                game.isOptionAvailable() ? std::cout << "\nType 2 to end turn, 1 to continue selecting or 0 to roll again: " : std::cout << "\nType 2 to end turn or 0 to roll again: ";
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<int>::max(), '\n');
                std::cin >> playOrEndTurn;

                while (std::cin.fail() || playOrEndTurn < 0 || 2 < playOrEndTurn) {
                    zilch::clear();
                    std::cout << "Type 2 to end turn, 1 to continue selecting or 0 to roll again: ";
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<int>::max(), '\n');
                    std::cin >> playOrEndTurn;
                }

                if ((playOrEndTurn == 2) && (game.getRunningScore() >= 1000))
                    applyAllPossibleOptions(game);

                game.setContinueTurnBool(playOrEndTurn != 2);
                game.setContinueSelectingBool(playOrEndTurn == 1);
                zilch::clear();
            }
        } else if (!game.isOptionAvailable()) {
            zilch::clear();
            zilch::showDice(game);
            std::cout << "\nThere are no options left";
            zilch::pauseAndContinue(game);
            break;
        }
    } while (game.getContinueTurnBool() && game.getContinueSelectingBool() && game.isOptionAvailable());
}

void Checker::straits(zilch& game) {
    if (!game.isStrait())
        return;

    game.setScoreFromSingles(1000);
    game.diceSetMap.clear();
    game.setOptionSelectedBool(true);
}

void Checker::set(zilch& game) {
    if (!game.isSet())
        return;

    game.setScoreFromSingles(1000);
    game.diceSetMap.clear();
    game.setOptionSelectedBool(true);
}

void Checker::multiple(zilch& game, uint8_t val) {
    if (!game.isDesiredMultipleAvailable(val) && !game.getValOfAvailableMultiple())
        return;

    if (game.isMultiple()) {
        game.setValOfChosenMultiple(val);
        uint16_t baseScore = (val == 1) ? 1000 : val * 100;
        uint8_t numMultiples = game.diceSetMap[val] - 3;
        uint16_t score = static_cast<uint16_t>(pow(2, numMultiples) * baseScore);

        if (game.diceSetMap[val] > 6 || game.diceSetMap[val] - 3 < 0)
            throw std::out_of_range("Error: You have " + std::to_string(game.diceSetMap[val]) + " dice. \nYou cannot have more than 6 dice or less than 3 multiples.");

        if (std::any_of(game.diceSetMap.begin(), game.diceSetMap.end(), [val](const auto& die) { return die.first == val; }))
            game.setScoreFromMultiples(score);
    } else if (game.canAddMultiples()) {
        if ((game.getScoreFromMultiples() < 200) || (game.diceSetMap[val] > 3))
            throw std::out_of_range("Error: You must have had a multiple selected and cannot have more than 6 dice or than 3 added multiples");

        if (std::any_of(game.diceSetMap.begin(), game.diceSetMap.end(), [val](const auto& die) { return die.first == val; }))
            game.setScoreFromMultiples(game.diceSetMap[val]);
    } else
        return;

    game.diceSetMap[val] = 0;
    game.setNumOfDiceInPlay();
    game.setOptionSelectedBool(true);
}

void Checker::single(zilch& game, uint8_t val) {
    if (!((val == 1 && game.isSingle(val)) || (val == 5 && game.isSingle(5))))
        return;

    if ((game.getValOfChosenMultiple() == val) || (game.diceSetMap[val] >= 3)) {
        std::cout << "The option to chose multiples has been automatically applied.\n" << std::endl;
        game.multiple(game, val);
    } else if ((game.diceSetMap[val] < 3) && (game.diceSetMap[val] >= 1)) {
        game.setScoreFromSingles((val == 1) ? 100 : 50);
        game.diceSetMap[val]--;
        game.setNumOfDiceInPlay(-1);
        game.setOptionSelectedBool(true);
    }
}

void Checker::updateValOfAvailableMultiples(zilch& game) {
    for (const auto& die : game.diceSetMap) {
        if (die.second >= 3)
            game.setValOfAvailableMultiple(die.first);
    }
}

void Checker::applyAllPossibleOptions(zilch& game) {
    while (game.isOptionAvailable()) {
        if (game.isStrait())
            game.straits(game);
        if (game.isSet())
            game.set(game);
        for (uint8_t i = 1; i <= 6; i++) {
            if ((game.isMultiple() && game.isDesiredMultipleAvailable(i)) || ((game.getScoreFromMultiples() >= 200) && (game.getValOfChosenMultiple() == i) && (game.canAddMultiples())))
                game.multiple(game, i);
        }
        if (game.isSingle(1))
            game.single(game, 1);
        if (game.isSingle(5))
            game.single(game, 5);
    }
}

void Checker::lastTurnOpportunity(zilch& game, int8_t FULL_SET_OF_DICE) {
    std::string gameEndingPlayer = game.getCurrentPlayer();

    std::cout << gameEndingPlayer << " is over " << game.getScoreLimit() << "\nEveryone else has one more chance to win" << std::endl;
    zilch::pauseAndContinue(game);
    std::cout << "\n" << std::endl;

    game.incCurrentPlayer();

    do {
        std::cout << "It is " << game.getCurrentPlayer() << "'s last turn" << std::endl;
        game.setNumOfDiceInPlay(FULL_SET_OF_DICE);

        while (game.getContinueTurnBool())
            game.rollSixDice(game);

        game.incCurrentPlayer();
    } while (game.getCurrentPlayer() != gameEndingPlayer);
}

void Checker::tiedEnding(zilch& game) {
    uint8_t tieCounter = 0;
    std::string playerWithHighestScore = game.getHighestScoringPlayer();
    uint32_t highestScore = game.getPermanentScore(playerWithHighestScore);
    std::vector<std::string> tie;
    tie.resize(game.getAmountOfPlayers());
    tie.at(0) = { playerWithHighestScore };

    for (const auto& player : game.getPlayers()) {
        if (game.getPermanentScore(player) == highestScore)
            tie.at(++tieCounter) = { player };
    }

    if (tieCounter > 1) {
        for (const auto& player : tie) {
            std::cout << player;
            if (player != tie.at(tie.size() - 2))
                std::cout << ", ";
            else if (player != tie.at(tie.size() - 1))
                std::cout << " and ";
        }
        std::cout << " have tied with " << highestScore << " Points!" << std::endl;
    } else
        std::cout << playerWithHighestScore << " won with " << highestScore << " Points!" << std::endl;
}
