#include "Zilch.h"
#include "Dice.h"
#include "Checker.h"
#include <numeric>

void Zilch::playGame() {
    initializeGame();
    printGameInfo();
    Zilch game = *this;
    Checker gameChecker(Zilch); // FIXME: May need to use game instead of ZIlch, but produces weird error
    Checker checker;

    while (true) {
        std::cout << "Enter the number of players (1-6): ";
        uint8_t numPlayers;
        std::cin >> numPlayers;

        if (1 <= numPlayers && numPlayers <= 6) {
            for (uint8_t i = 0; i < numPlayers; i++) {
                std::string playerName;
                std::cout << "Enter the name of player " << i + 1 << ": ";
                std::cin >> playerName;
                addPlayer(playerName);
            }
            break;
        } else {
            std::cout << "Invalid number of players. Please try again." << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<int>::max(), '\n');
        }
    }

    while (true) {
        std::cout << "\nEnter the score limit (minimum 1000): ";
        std::cin >> currentPlayer->score.scoreLimit;

        if (currentPlayer->score.scoreLimit >= 1000)
            break;
        else std::cout << "Invalid score limit. Please try again." << std::endl;
    }

    std::cout << "\nGame start!\n" << std::endl;
    pauseAndContinue(game, true);

    while (true) {
        for (const auto &player: players) {
            if (player.score.permanentScore >= player.score.scoreLimit) {
                checker.lastTurnOpportunity(); // FIXME: May need to use gameChecker instead of checker, but produces weird error
                checker.tiedEnding();
                return;
            }

            *currentPlayer = player;
            while (continueTurnBool)
                Dice::rollSixDice(game, gameChecker);

            pauseAndContinue(game, true);
        }
    }
}

void Zilch::initializeGame() {
    players.clear();
    currentPlayer->dice.diceSetMap.clear();
    // numOfDiceInPlay = 6;
    currentPlayer->score.turnScore = 0;
    currentPlayer->score.runningScore = 0;
    currentPlayer->score.scoreFromMultiples = 0;
    currentPlayer->score.scoreFromSingles = 0;
    valOfChosenMultiple = 0;
    valOfAvailableMultiple = 0;
    optionSelectedBool = false;
    continueTurnBool = false;
    continueSelectingBool = false;
}

void Zilch::printGameInfo() {
    std::cout << "Welcome to Zilch!\n" << std::endl
              << "Here are the basic rules:" << std::endl
              << "You must score an initial 1000 points to start logging your points" << std::endl
              << "Sets (three sets of two) and straits (1, 2, 3, 4, 5, 6) give 1000 points" << std::endl
              << "A group of 3 dice give you 100 points times the value of that die, for example:" << std::endl
              << "\ta roll of 3 3 3 will give you 300 points" << std::endl
              << "\tan exception is made for 1 1 1 which will give you 1000 points" << std::endl
              << "Each additional number added to this group of three doubles the points received from it, for example:" << std::endl
              << "\ta roll of 3 3 3 3 or a roll of 3 3 3 and a later roll of 3 in the same turn would give you 600 points" << std::endl
              << "\tthe same applies to a group of 3 1's, 2 more 1's are rolled the score would be 1000*2*2 = 4000" << std::endl
              << "Finally, only single 1's or 5's are worth points with a single 1 being 100 points and a 5 being 50." << std::endl;
}

void Zilch::addPlayer(const Player &player) {
    players.push_back(player);
}

Player* Zilch::getHighestScoringPlayer() {
    if (players.empty())
        throw std::runtime_error("No players available");

    return &*std::max_element(players.begin(), players.end(),
                             [](const Player &p1, const Player &p2) {
                                 return p1.score.permanentScore < p2.score.permanentScore;
                             });
}

/*size_t Zilch::getAmountOfPlayers() const {
    return players.size();
}*/

/*uint8_t Zilch::getNumOfDiceInPlay() const {
    return numOfDiceInPlay;
}*/

void Zilch::setNumOfDiceInPlay(int8_t numOfDice) {
    bool repeat;
    do {
        repeat = false;
        for (const auto die: currentPlayer->dice.diceSetMap)
            if (die.second == 0) {
                currentPlayer->dice.diceSetMap.erase(die.first);
                repeat = true;
                break;
            }
    } while (repeat);


    if (numOfDice > 6)
        numOfDiceInPlay = 6;
    else if (numOfDice == -20) // Recount the number of dice available
    {
        numOfDiceInPlay = 0;
        for (const auto die: diceSetMap)
            numOfDiceInPlay += die.second;
    } else if (numOfDice < 0) { // (-1) Subtract or reset dice
        numOfDiceInPlay = (abs(numOfDice) <= numOfDiceInPlay) ? numOfDiceInPlay + numOfDice : 6;
    } else if ((numOfDice == 0) || (numOfDice == 6)) // New roll or new turn
    {
        setValOfChosenMultiple(0);

        setTurnScores(getScoreFromMultiples() + getScoreFromSingles());
        numOfDiceInPlay = numOfDice;
    } else numOfDiceInPlay = numOfDice;

    if (numOfDice == 6)
        continueTurnBool = true; // Using the set function creates a recursive call chain
}

uint32_t Zilch::getTurnScore() const {
    return turnScore;
}

void Zilch::setTurnScores(uint32_t score) {
    currentPlayer->score.turnScore = score;
}

uint32_t Zilch::getRunningScore() const {
    return currentPlayer->score.runningScore;
}

void Zilch::setRunningScore(uint32_t score) {
    runningScore = score;
}

uint32_t Zilch::getScoreFromMultiples() const {
    return scoreFromMultiples;
}

void Zilch::setScoreFromMultiples(uint32_t score) {
    scoreFromMultiples = score;
}

uint32_t Zilch::getScoreFromSingles() const {
    return scoreFromSingles;
}

void Zilch::setScoreFromSingles(uint32_t score) {
    scoreFromSingles = score;
}

uint32_t Zilch::getScoreLimit() const {
    return scoreLimit;
}

uint8_t Zilch::getValOfChosenMultiple() const {
    return valOfChosenMultiple;
}

void Zilch::setValOfChosenMultiple(uint8_t val) {
    valOfChosenMultiple = val;
}

uint8_t Zilch::getValOfAvailableMultiple() const {
    return valOfAvailableMultiple;
}

void Zilch::setValOfAvailableMultiple(uint8_t val) {
    valOfAvailableMultiple = val;
}

bool Zilch::isOptionAvailable() const {
    return (isStrait() || isSet() || canAddMultiples() || isSingle(1) || isSingle(5));
}

bool Zilch::getOptionSelectedBool() const {
    return optionSelectedBool;
}

void Zilch::setOptionSelectedBool(bool value) {
    optionSelectedBool = value;
}

bool Zilch::getContinueTurnBool() const {
    return continueTurnBool;
}

bool Zilch::getContinueSelectingBool() const {
    return continueSelectingBool;
}

void Zilch::setContinueTurnBool(bool turnBool, bool selectingBool) {
    continueTurnBool = turnBool;
    continueSelectingBool = selectingBool;
}

void pauseAndContinue(const Zilch &game, bool showCurrentScore) {
    std::cout << "\nPress enter to continue ..." << std::flush;
    if (strcmp(PLATFORM_NAME, "windows") > 0)
        system("pause"); // For Windows users
    else if (strcmp(PLATFORM_NAME, "macos") > 0)
        system("read"); // For  macOS users

    clear();

    if (showCurrentScore)
        std::cout << game.getCurrentPlayer() << "'s current score: " << game.getRunningScore() << std::endl;
}

void showDice(const Zilch &game) {
    std::cout << "\nYou have " << game.getNumOfDiceInPlay() << " dice left" << std::endl << game.getCurrentPlayer() << "'s current score: "
              << game.getRunningScore() << std::endl;

    ///   Print dice vector   ///
    std::map<uint8_t, uint8_t> diceSetMap = game.getDiceMap();
    for (const auto &die: diceSetMap)
        for (uint8_t i = 0; i < die.second; i++)
            if (die.first == diceSetMap.rbegin()->first && i == die.second - 1) // If it is on the very last element
                std::cout << die.first << std::endl;
            else std::cout << die.first << ", ";
    game.updateDiceMap(diceSetMap);
}

void clear() {
    std::cout << "\033[2J\033[1;1H";
    /*
     	if (strcmp(PLATFORM_NAME, "windows") > 0)
		system("cls");
	else if (strcmp(PLATFORM_NAME, "macos") > 0)
		system("clear");

/*        if (!cur_term) {
            int result;
            setupterm(nullptr, STDOUT_FILENO, &result);
            //if (result <= 0) return;
        }
        putp(tigetstr((char *) "clear"));
    }*/
}

void printInstructions(const Zilch &game, int options) {
    //Zilch::showDice(game);

    switch (options) {
        case Zilch::ENTER:
            std::cout << "\tSingles -- s#, \n\tMultiples -- m#, \n\tSet -- se, \n\tStrait -- st;" << std::endl
                      << ((game.getOptionSelectedBool()) ? (game.getRunningScore() >= 1000) ? "Enter the option you wish to take, or type 0 to end your turn: "
                                                                                            : "Enter the option you wish to take, or type 0 to roll again: "
                                                         : "Enter the option you wish to take: ") << std::flush;
            break;

        case Zilch::NEXT:
            std::cout << "\tSingles -- s#, \n\tMultiples -- m#, \n\tSet -- se, \n\tStrait -- st;" << std::endl
                      << ((game.getOptionSelectedBool()) ? (game.getRunningScore() >= 1000) ? "Please enter your next choice, or type 0 to end your turn: "
                                                                                            : "Please enter your next choice, or type 0 to roll again: "
                                                         : "Please enter your next choice: ") << std::flush;
            break;

        case Zilch::REENTER:
            std::cout << "\tSingles -- s#, \n\tMultiples -- m#, \n\tSet -- se, \n\tStrait -- st;" << std::endl
                      << ((game.getOptionSelectedBool()) ? (game.getRunningScore() >= 1000)
                                                           ? "Please re-enter the option you wish to take, or type 0 to end your turn: "
                                                           : "Please re-enter the option you wish to take, or type 0 to roll again: "
                                                         : "Please re-enter the option you wish to take: ") << std::flush;
            break;

        default:
            throw std::invalid_argument("Invalid enumeration, File: " + std::string(__FILE_NAME__) + ", Line: " + std::to_string(__LINE__));
            break;
    }
}

std::istream &operator>>(std::istream &input, Zilch &game) {
    u_char ch = '\0';
    uint8_t val = 0;

    input >> ch;


    while ((ch == ' ') ||
           ((ch != 's') && (ch != 't') && (ch != 'e') && (ch != 'l') && (ch != '1') && (ch != '2') && (ch != '3') && (ch != '4') && (ch != '5') &&
            (ch != '6') && (ch != 'a') && (ch != 'm') && (ch != '0') && (ch != '?')))
        input >> ch;

    switch (ch) {
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
            val = ch - 48; // Subtract 48 to convert (in ASCII) from '1' to 1, NOTE: must make sure ch > 48
            if (game.isStrait())
                Checker::straits(game);
            else if (game.isSet())
                Checker::set(game);
            else if ((game.isMultiple() && game.isDesiredMultipleAvailable(val)) // Multiples available
                     || ((game.getScoreFromMultiples() >= 200) && (game.getValOfChosenMultiple() == val) &&
                         (game.canAddMultiples()))) // Adding multiples available
                Checker::multiple(game, val);
            else if ((val == 1 || val == 5) && game.isSingle(val))
                Checker::single(game, val);
            else {
                clear();
                std::cout << "You have selected an impossible option, please select again" << std::endl;
            }
            break;

        case 's':
            input >> ch;
            if ((ch == 't') && game.isStrait())
                Checker::straits(game);
            else if ((ch == 'e') && game.isSet())
                Checker::set(game);
            else {
                if ((ch == '1') && game.isSingle(1))
                    Checker::single(game, 1);
                else if ((ch == '5') && game.isSingle(5))
                    Checker::single(game, 5);
                else {
                    clear();
                    std::cout << "You have selected an impossible option, please select again" << std::endl;
                }
            }
            break;

        case 'a':
            input >> ch;
            if (input.fail()) {
                input.clear();
                input.ignore(std::numeric_limits<int>::max(), '\n');
                [[fallthrough]];
            } else if (ch == 'l') {
                Checker::applyAllPossibleOptions(game);
                break;
            }

        case 'm':
            input >> val;
            if ((game.isMultiple() && game.isDesiredMultipleAvailable(val)) // Multiples available
                || ((game.getScoreFromMultiples() >= 200) && (game.getValOfChosenMultiple() == val) && (game.canAddMultiples()))) // Adding multiples available
                Checker::multiple(game, val);
            else {
                std::cout << "You have selected an impossible option" << std::endl;
                Checker::updateValOfAvailableMultiples(game);
                if (game.getValOfChosenMultiple() != 0)
                    std::cout << "The Value of the multiple is: " << game.getValOfChosenMultiple() << std::endl;
                else if (game.getValOfAvailableMultiple() != 0)
                    std::cout << "The Value of the multiple is: " << game.getValOfAvailableMultiple() << std::endl;
                else {
                    clear();
                    std::cout << "There is no available multiple" << std::endl;
                }
            }
            break;

        case '0':
            if (game.getOptionSelectedBool())
                game.setContinueSelectingBool(false);
            else if ((game.getRunningScore() >= 1000) && game.isOptionAvailable()) {
                game.setPermanentScore(game.getScoreFromSingles());
                game.setScoreFromSingles(0);
                std::cout << "Your official score is now: " << game.getPermanentScore() << std::endl;
                game.setNumOfDiceInPlay(6);
                game.incCurrentPlayer();
                std::cout << "\nIt is now " << game.getCurrentPlayer() << "'s turn" << std::endl;
            } else if (!game.isOptionAvailable()) {
                clear();
                return input;
            } else {
                clear();
                std::cout << "You cannot end without a score higher than 1000" << std::endl;
                game.pauseAndContinue(game, true);
            }
            break;

        case '?':
            std::cout << "Possible options: " << std::endl;
            if (game.isStrait())
                std::cout << "\tStrait" << std::endl;
            if (game.isSet())
                std::cout << "\tSets" << std::endl;
            if (game.isMultiple())
                std::cout << "\tMultiples, value: " << game.getValOfAvailableMultiple() << std::endl;
            if (game.canAddMultiples())
                std::cout << "\tAddons, value: " << game.getValOfChosenMultiple() << std::endl;
            if (game.isSingle(1) || game.isSingle(5))
                std::cout << "\tSingles" << std::endl;
            break;

        default:
            clear();
            std::cout << "You have selected an impossible option" << std::endl;
            break;
    }

    return input;
}
