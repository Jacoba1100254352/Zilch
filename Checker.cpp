#include "Checker.h"
#include "Dice.h"
#include <iostream>
#include <cmath>
#include <limits>

void Checker::check() {
    if (!game.isOptionAvailable()) {
        if (game.getNumOfDiceInPlay() == 6 && !game.getTurnScore()) {
            showDice(game);
            std::cout << "\nYou have busted on the first roll, try again" << std::endl;
            game.setScoreFromSingles(50);
            pauseAndContinue(game, true);
        } else if (game.getNumOfDiceInPlay() >= 1) {
            showDice(game);
            std::cout << "\nYou have busted" << std::endl;
            game.setNumOfDiceInPlay(0);
            game.setTurnScores(0);
            game.setContinueTurnBool(false, true);
            pauseAndContinue(game);
        } else {
            std::cout << "\nYou have a full set of dice now" << std::endl;
            game.setNumOfDiceInPlay(6);
            pauseAndContinue(game, true);
        }
    } else checkingUserInput();
}

void Checker::checkingUserInput() {
    uint8_t playOrEndTurn = 0;

    do {
        clear();
        updateValOfAvailableMultiples();

        if (canAddMultiples() && (game.getScoreFromMultiples() >= 200))
            std::cout << "You can add to your multiple of " << game.getValOfChosenMultiple() << "!\n";

        if (!game.getOptionSelectedBool())
            printInstructions(game, Zilch::ENTER);
        else if (game.isOptionAvailable())
            printInstructions(game, Zilch::NEXT);
        else break;

        std::cin.clear();
        std::cin.ignore(std::numeric_limits<int>::max(), '\n');
        std::cin >> game;

        if (game.getRunningScore() >= 1000) {
            uint32_t highestScore = game.getHighestScoringPlayer()->score.permanentScore;

            if (highestScore < game.getScoreLimit())
                std::cout << "\n" << game.getCurrentPlayer() << "'s current score: " << game.getRunningScore();
            else if (highestScore > game.getRunningScore())
                std::cout << "\n\nYour current score of " << game.getRunningScore() << " is " << highestScore - game.getRunningScore() << " less than " << game.getHighestScoringPlayer()->name << "'s High Score of " << highestScore << " so keep going! :)" << std::endl;
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
                    clear();
                    std::cout << "Type 2 to end turn, 1 to continue selecting or 0 to roll again: ";
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<int>::max(), '\n');
                    std::cin >> playOrEndTurn;
                }

                if ((playOrEndTurn == 2) && (game.getRunningScore() >= 1000))
                    applyAllPossibleOptions();

                game.setContinueTurnBool(playOrEndTurn != 2);
                game.setContinueSelectingBool(playOrEndTurn == 1);
                clear();
            }
        } else if (!game.isOptionAvailable()) {
            clear();
            showDice(game);
            std::cout << "\nThere are no options left";
            pauseAndContinue(game);
            break;
        }
    } while (game.getContinueTurnBool() && game.getContinueSelectingBool() && game.isOptionAvailable());
}

void Checker::straits() {
    if (!isStrait())
        return;

    game.setScoreFromSingles(1000);
    game.getCurrentPlayer()->dice.diceSetMap.clear();
    game.setOptionSelectedBool(true);
}

void Checker::set() {
    if (!isSet())
        return;

    game.setScoreFromSingles(1000);
    game.getCurrentPlayer()->dice.diceSetMap.clear();
    game.setOptionSelectedBool(true);
}

void Checker::multiple(uint8_t val) {
    std::map<uint8_t, uint8_t> &_diceSetMap = game.getCurrentPlayer()->dice.diceSetMap;
    if (!isDesiredMultipleAvailable(val) && !game.getValOfAvailableMultiple())
        return;

    if (isMultiple()) {
        game.setValOfChosenMultiple(val);
        uint16_t baseScore = (val == 1) ? 1000 : val * 100;
        uint8_t numMultiples = _diceSetMap[val] - 3;
        auto score = static_cast<uint16_t>(pow(2, numMultiples) * baseScore);

        if (_diceSetMap[val] > 6 || _diceSetMap[val] - 3 < 0)
            throw std::out_of_range("Error: You have " + std::to_string(_diceSetMap[val]) + " dice. \nYou cannot have more than 6 dice or less than 3 multiples.");

        if (std::any_of(_diceSetMap.begin(), _diceSetMap.end(), [val](const auto& die) { return die.first == val; }))
            game.setScoreFromMultiples(score);
    } else if (canAddMultiples()) {
        if ((game.getScoreFromMultiples() < 200) || (_diceSetMap[val] > 3))
            throw std::out_of_range("Error: You must have had a multiple selected and cannot have more than 6 dice or than 3 added multiples");

        if (std::any_of(_diceSetMap.begin(), _diceSetMap.end(), [val](const auto& die) { return die.first == val; }))
            game.setScoreFromMultiples(_diceSetMap[val]);
    } else
        return;

    _diceSetMap[val] = 0;
    game.setNumOfDiceInPlay();
    game.setOptionSelectedBool(true);
}

void Checker::single(uint8_t val) {
    std::map<uint8_t, uint8_t> &_diceSetMap = game.getCurrentPlayer()->dice.diceSetMap;
    if (!((val == 1 && isSingle(val)) || (val == 5 && isSingle(5))))
        return;

    if ((game.getValOfChosenMultiple() == val) || (_diceSetMap[val] >= 3)) {
        std::cout << "The option to chose multiples has been automatically applied.\n" << std::endl;
        multiple(val);
    } else if ((_diceSetMap[val] < 3) && (_diceSetMap[val] >= 1)) {
        game.setScoreFromSingles((val == 1) ? 100 : 50);
        _diceSetMap[val]--;
        game.setNumOfDiceInPlay(-1);
        game.setOptionSelectedBool(true);
    }
}

void Checker::updateValOfAvailableMultiples() {
    std::map<uint8_t, uint8_t> &_diceSetMap = game.getCurrentPlayer()->dice.diceSetMap;
    for (const auto& die : _diceSetMap)
        if (die.second >= 3)
            game.setValOfAvailableMultiple(die.first);
}

void Checker::applyAllPossibleOptions() {
    while (game.isOptionAvailable()) {
        if (isStrait())
            straits();
        if (isSet())
            set();
        for (uint8_t i = 1; i <= 6; i++)
            if ((isMultiple() && isDesiredMultipleAvailable(i)) || ((game.getScoreFromMultiples() >= 200) && (game.getValOfChosenMultiple() == i) && (canAddMultiples())))
                multiple(i);

        if (isSingle(1))
            single(1);
        if (isSingle(5))
            single(5);
    }
}

void Checker::lastTurnOpportunity(const uint8_t FULL_SET_OF_DICE) {
    Player* gameEndingPlayer = game.getCurrentPlayer();

    std::cout << gameEndingPlayer->name << " is over " << game.getScoreLimit() << "\nEveryone else has one more chance to win" << std::endl;
    pauseAndContinue(game);
    std::cout << "\n" << std::endl;

    game.incCurrentPlayer();

    do {
        std::cout << "It is " << game.getCurrentPlayer()->name << "'s last turn" << std::endl;
        game.setNumOfDiceInPlay((int8_t) FULL_SET_OF_DICE);

        Checker checker(game);
        while (game.getContinueTurnBool())
            Dice::rollSixDice(game, checker);

        game.incCurrentPlayer();
    } while (game.getCurrentPlayer() != gameEndingPlayer);
}

void Checker::tiedEnding() {
    uint8_t tieCounter = 0;
    Player* playerWithHighestScore = game.getHighestScoringPlayer();
    uint32_t& highestScore = playerWithHighestScore->score.permanentScore;
    std::vector<Player*> tie;

    for (auto& player : game.getPlayers())
        if (player.score.permanentScore == highestScore)
            tie.push_back(&player);

    if (tie.size() > 1) {
        for (size_t i = 0; i < tie.size(); ++i) {
            std::cout << tie[i]->name;
            if (i != tie.size() - 1) {
                std::cout << ((i == tie.size() - 2) ? " and " : ", ");
            }
        }
        std::cout << " have tied with " << highestScore << " Points!" << std::endl;
    } else std::cout << playerWithHighestScore->name << " won with " << highestScore << " Points!" << std::endl;
}

bool Checker::isStrait() {
    return (game.getCurrentPlayer()->dice.diceSetMap.size() == 6);
}

bool Checker::isSet() {
    return (game.getCurrentPlayer()->dice.diceSetMap.size() == 3 && !isStrait() && !isMultiple() &&
            !std::any_of(game.getCurrentPlayer()->dice.diceSetMap.begin(), game.getCurrentPlayer()->dice.diceSetMap.end(), [](const auto &die) { return die.second != 2; }));
}

bool Checker::isDesiredMultipleAvailable(uint8_t desiredMultiple) {
    return (std::any_of(game.getCurrentPlayer()->dice.diceSetMap.begin(), game.getCurrentPlayer()->dice.diceSetMap.end(), [desiredMultiple](const auto &die) {
        return die.first == desiredMultiple && die.second >= 3;
    }));
}

///   Checks to verify that Multiples exist   ///
bool Checker::isMultiple() { // or is/areMultiples
    return (std::any_of(game.getCurrentPlayer()->dice.diceSetMap.begin(), game.getCurrentPlayer()->dice.diceSetMap.end(), [](const auto &die) { return die.second >= 3; }));
}

bool Checker::canAddMultiples() {
    return (std::any_of(game.getCurrentPlayer()->dice.diceSetMap.begin(), game.getCurrentPlayer()->dice.diceSetMap.end(), [this](const auto &die) { return die.first == game.getValOfChosenMultiple(); }));
}

bool Checker::isSingle(uint8_t single) {
    return (std::any_of(game.getCurrentPlayer()->dice.diceSetMap.begin(), game.getCurrentPlayer()->dice.diceSetMap.end(), [single](const auto &die) { return die.first == single; }));
}
