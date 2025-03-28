// GameManager.cpp
#include "GameManager.h"
#include <algorithm>
#include <random>
#include "Checker.h"

////////////////////////////////////
//        UTILITY FUNCTIONS
////////////////////////////////////
static void clearScreen() {
    std::cout << "\033[2J\033[1;1H";
}

static void pauseAndContinue() {
    std::cout << "Press ENTER to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

////////////////////////////////////
//       Dice Implementation
////////////////////////////////////
void Dice::rollDice() {
    diceSetMap.clear();
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<> dist(1, 6);

    for (int i = 0; i < numDiceInPlay; ++i) {
        auto val = static_cast<uint16_t>(dist(mt));
        diceSetMap[val]++;
    }
}

void Dice::displayDice() const {
    std::cout << "Dice in play: " << numDiceInPlay << "\n";
    std::cout << "Rolled: ";
    for (const auto & [fst, snd] : diceSetMap) {
        for (int i = 0; i < snd; i++) {
            std::cout << fst << ' ';
        }
    }
    std::cout << "\n";
}

////////////////////////////////////
//      GameManager Implementation
////////////////////////////////////
GameManager::GameManager()
    : currentState(ZilchGameState::SETUP),
      currentPlayerIndex(0)
{
    // Possibly do other init here...
}

Player& GameManager::getCurrentPlayer() {
    return players[currentPlayerIndex];
}

void GameManager::switchToNextPlayer() {
    currentPlayerIndex = static_cast<int>((currentPlayerIndex + 1) % players.size());
}

bool GameManager::isGameOver() const {
    // Example: If any player meets or exceeds score limit, set game over.
    // In practice, you might store a boolean or a dedicated check.
    return std::ranges::any_of(players, [](const Player& p) {
        return p.score.perm >= p.score.limit;
    });
}

////////////////////////////////////
//       Main Run Function
////////////////////////////////////
void GameManager::run() {
    // The main game loop: keep transitioning states until GAME_OVER
    while (currentState != ZilchGameState::GAME_OVER) {
        switch (currentState) {
            case ZilchGameState::SETUP:          doSetupState();      break;
            case ZilchGameState::BEGIN_TURN:     doBeginTurn();       break;
            case ZilchGameState::ROLL_DICE:      doRollDice();        break;
            case ZilchGameState::CHECK_OPTIONS:  doCheckOptions();    break;
            case ZilchGameState::SELECT_OPTION:  doSelectOption();    break;
            case ZilchGameState::APPLY_SCORE:    doApplyScore();      break;
            case ZilchGameState::END_TURN:       doEndTurn();         break;
            case ZilchGameState::CHECK_GAME_END: doCheckGameEnd();    break;
            case ZilchGameState::LAST_TURN:      doLastTurn();        break;
            case ZilchGameState::GAME_OVER:      /* no-op */          break;
        }
    }

    clearScreen();
    std::cout << "Game Over! Final scores:\n";
    for (auto &p : players) {
        std::cout << p.name << " => " << p.score.perm << "\n";
    }
    std::cout << "Thanks for playing!\n";
}

////////////////////////////////////
//         State Handlers
////////////////////////////////////

/**
 * SETUP: Let user enter # of players, names, and the score limit.
 */
void GameManager::doSetupState() {
    clearScreen();
    std::cout << "\nHow many players? (1-6): ";
    int n = 0;
    std::cin >> n;
    if(!std::cin || n < 1 || n > 6) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid number, try again.\n";
        return;
    }

    players.clear();
    for(int i = 1; i <= n; i++) {
        std::string name;
        std::cout << "Enter name for player " << i << ": ";
        std::cin >> name;
        players.emplace_back(name);
    }

    // Score limit
    uint32_t limit;
    std::cout << "Enter score limit (>=1000): ";
    std::cin >> limit;
    if (!std::cin || limit < 1000) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid limit, setting to 2000.\n";
        limit = 2000;
    }

    for (auto &p : players) {
        p.score.limit = limit;
    }
    pauseAndContinue();
    currentState = ZilchGameState::BEGIN_TURN;
}

/**
 * BEGIN_TURN: Reset round score to 0, handle any pre-roll logic
 */
void GameManager::doBeginTurn() {
    clearScreen();
    Player &p = getCurrentPlayer();
    std::cout << "======== BEGIN TURN ========\n";
    std::cout << p.name << "'s turn!\n";
    p.score.round = 0;       // reset round score
    p.score.multiple = 0;    // reset multiple
    p.dice.setNumDiceInPlay(6); // restore to 6 dice if needed

    pauseAndContinue();
    currentState = ZilchGameState::ROLL_DICE;
}

/**
 * ROLL_DICE: Actually roll
 */
void GameManager::doRollDice() {
    clearScreen();
    Player &p = getCurrentPlayer();
    p.dice.rollDice();
    p.dice.displayDice();

    pauseAndContinue();
    currentState = ZilchGameState::CHECK_OPTIONS;
}

/**
 * CHECK_OPTIONS: Evaluate possible scoring/bust
 * (Here we might use `Checker` to see if any scoring combos exist.)
 */
void GameManager::doCheckOptions() {
    clearScreen();
    // For brevity, just pretend we check bust conditions, etc.
    // If bust: round=0 => go to END_TURN
    // Otherwise => SELECT_OPTION
    const Checker c(*this);

    // Example check (not a full logic):
    const bool hasOption = c.hasValidOption();
    const bool firstRollBust = (getCurrentPlayer().dice.getNumDiceInPlay() == 6
                          && getCurrentPlayer().score.round == 0
                          && !hasOption);

    if (firstRollBust) {
        std::cout << "Oh no, first roll bust!\n";
        c.handleBust(/*isFirstRoll=*/true);
        currentState = ZilchGameState::END_TURN;
    }
    else if(!hasOption) {
        std::cout << "Bust! No valid moves.\n";
        c.handleBust(/*isFirstRoll=*/false);
        currentState = ZilchGameState::END_TURN;
    }
    else {
        std::cout << "You have some valid scoring options.\n";
        pauseAndContinue();
        currentState = ZilchGameState::SELECT_OPTION;
    }
}

/**
 * SELECT_OPTION: The player chooses what to do (take single, multiple, etc.) or end turn
 * If they choose a scoring action, transition to APPLY_SCORE
 * If they want to end turn, go to END_TURN
 * If they want to roll again, go back to ROLL_DICE
 */
void GameManager::doSelectOption() {
    clearScreen();
    Player &p = getCurrentPlayer();
    std::cout << "======== SELECT OPTION ========\n";
    std::cout << p.name << "'s current round score: " << p.score.round << "\n";

    std::cout << "Menu:\n";
    std::cout << "1) Take Single (1 or 5)\n";
    std::cout << "2) Take Multiple\n";
    std::cout << "3) End Turn\n";
    std::cout << "4) Roll Again\n";
    std::cout << "Choice: ";

    int choice;
    std::cin >> choice;
    if(!std::cin) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        choice = 0;
    }

    switch(choice) {
    case 1:
    case 2:
        // We’ll do actual scoring in APPLY_SCORE.
        currentState = ZilchGameState::APPLY_SCORE;
        break;
    case 3:
        // End turn
        currentState = ZilchGameState::END_TURN;
        break;
    case 4:
        // Roll again
        currentState = ZilchGameState::ROLL_DICE;
        break;
    default:
        std::cout << "Invalid choice, try again.\n";
        pauseAndContinue();
        // remain in SELECT_OPTION
        break;
    }
}

/**
 * APPLY_SCORE: Actually apply the chosen score, remove dice, etc.
 */
void GameManager::doApplyScore() {
    clearScreen();
    Checker c(*this);

    // For brevity, let's just call some "simulate scoring" method
    // In practice, you'd ask the user what they chose: single, set, multiple, etc.
    // Then call c.handleSingle(dieValue) or c.handleMultiple(dieValue).

    // As an example:
    std::cout << "Applying a pretend +100 to round score.\n";
    getCurrentPlayer().score.round += 100;

    // Check if the player can continue selecting more combos or not...
    // If no dice remain or no combos remain => might end turn automatically, etc.

    pauseAndContinue();
    currentState = ZilchGameState::SELECT_OPTION;
    // or ROLL_DICE if you want them to roll again automatically
}

/**
 * END_TURN: If the player's round >= 1000, add to permanent. Move on.
 */
void GameManager::doEndTurn() {
    clearScreen();
    Player &p = getCurrentPlayer();
    // If we have 1000 or more for round, we store it permanently
    if (p.score.round >= 1000) {
        p.score.perm += p.score.round;
        std::cout << p.name << " banks " << p.score.round
                  << " points for a total of " << p.score.perm << "\n";
    } else {
        std::cout << p.name << " ended turn with < 1000 => no bank.\n";
    }
    p.score.round = 0;

    pauseAndContinue();
    currentState = ZilchGameState::CHECK_GAME_END;
}

/**
 * CHECK_GAME_END: See if the game is over, or if we proceed to next turn
 */
void GameManager::doCheckGameEnd() {
    if (isGameOver()) {
        // Optionally let every other player get a last turn => LAST_TURN
        currentState = ZilchGameState::LAST_TURN;
    } else {
        // Keep playing
        switchToNextPlayer();
        currentState = ZilchGameState::BEGIN_TURN;
    }
}

void GameManager::doLastTurn() {
    // Example: everyone but the leading player gets one more turn
    // For simplicity, we’ll just say “game over”
    std::cout << "Someone exceeded the limit => last turn triggered, but skipping for brevity.\n";
    pauseAndContinue();
    currentState = ZilchGameState::GAME_OVER;
}
