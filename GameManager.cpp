// GameManager.cpp
/**
Author: Jacob Anderson
Class: ECE-6122-A
Last Date Modified: 12/2/2025

Description:
This file contains the implementation of the GameManager class, which manages the overall game state, players, turns, and rules.
*/

#include "GameManager.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

GameManager::GameManager(const GameManager& other) : GameManager()
{
    *this = other;
}

GameManager& GameManager::operator=(const GameManager& other)
{
    if (this == &other)
        return *this;
    players = other.players;
    currentIndex = other.currentIndex;
    currentPlayer = other.currentPlayer && currentIndex < players.size() ? &players[currentIndex] : nullptr;
    valueOfChosenMultiple = other.valueOfChosenMultiple;
    selectedOptionStatus = other.selectedOptionStatus;
    turnContinuationStatus = other.turnContinuationStatus;
    selectionContinuationStatus = other.selectionContinuationStatus;
    bustPending = other.bustPending;
    computerBankCommitted = other.computerBankCommitted;
    computerSafeFinishCommitted = other.computerSafeFinishCommitted;
    computerJointBankDecision = other.computerJointBankDecision;
    computerJointSelections = other.computerJointSelections;
    hasRolledThisTurn = other.hasRolledThisTurn;
    bustBonusUsedThisTurn = other.bustBonusUsedThisTurn;
    rollCountThisTurn = other.rollCountThisTurn;
    finalRoundActive = other.finalRoundActive;
    finalRoundStarterIndex = other.finalRoundStarterIndex;
    finalRoundLeaderIndex = other.finalRoundLeaderIndex;
    stealOffer = other.stealOffer;
    stealContinuationActive = other.stealContinuationActive;
    ruleConfig = other.ruleConfig;
    stateMachine = other.stateMachine;
    return *this;
}

void GameManager::setComputerJointPlan(std::vector<std::size_t> selections, const bool bank)
{
    computerJointSelections = std::move(selections);
    computerJointBankDecision = bank;
}

void GameManager::clearComputerIntent()
{
    computerBankCommitted = false;
    computerSafeFinishCommitted = false;
    computerJointSelections.clear();
    computerJointBankDecision.reset();
}

/**
 * @brief Initializes the roll cycle by resetting relevant statuses and scores.
 */
void GameManager::initializeRollCycle(const bool resetTurnFlags)
{
    // Common resets for any roll-cycle start.
    turnContinuationStatus = true;
    bustPending = false;
    selectionContinuationStatus = true;

    // Only reset roll/selection state when starting a brand-new turn.
    if (resetTurnFlags) {
        clearComputerIntent();
        hasRolledThisTurn = false;
        selectedOptionStatus = false;
        valueOfChosenMultiple = 0;
        if (currentPlayer) currentPlayer->getScore().setScoreFromMultiples(0);
        bustBonusUsedThisTurn = false;
        rollCountThisTurn = 0;
    }
}

/**
 * @brief Manages the number of dice in play for the current player.
 *
 * @param numOfDice The number of dice to set in play. If zero, sets to full set.
 */
void GameManager::manageDiceCount(uint16_t numOfDice)
{
    // Ensure there is a current player.
    if (!currentPlayer)
        return;

    if (numOfDice == 0) {
        // Scoring every available die produces hot dice and ends any steal chain.
        endStealChain();
        numOfDice = FULL_SET_OF_DICE;
    }

    // Update the current player's dice count.
    currentPlayer->getDice().setNumDiceInPlay(numOfDice);

    // If resetting to full set, initialize roll cycle.
    if (numOfDice == FULL_SET_OF_DICE) {
        // Hot dice end the saved multiple immediately, including in look-ahead
        // before the UI has performed its next animated roll.
        resetSavedMultiple();
        initializeRollCycle(false);
    }
}

/**
 * @brief Sets the players for the game based on provided names.
 *
 * @param playerNames A vector of player names to initialize players.
 */
void GameManager::setPlayers(const std::vector<std::string>& playerNames)
{
    std::vector<PlayerSetup> playerSetups;
    playerSetups.reserve(playerNames.size());
    for (const auto& name : playerNames)
        playerSetups.push_back({name, PlayerKind::Human, ComputerDifficulty::Medium});
    setPlayerSetups(playerSetups);
}

/**
 * @brief Sets human and computer players from the setup screen.
 *
 * @param playerSetups The configured names, player kinds, and computer levels.
 */
void GameManager::setPlayerSetups(const std::vector<PlayerSetup>& playerSetups)
{
    clearComputerIntent();
    endStealChain();

    // Clear existing players and reserve space.
    players.clear();
    players.reserve(playerSetups.size());

    // Create players from provided setup records.
    for (const auto& setup : playerSetups)
        players.emplace_back(setup);

    // Set the first player as the current player.
    resetFinalRoundState();
    if (!players.empty()) {
        currentPlayer = &players.front();
        currentIndex = 0;
    } else {
        currentPlayer = nullptr;
        currentIndex = 0;
        valueOfChosenMultiple = 0;
        selectedOptionStatus = false;
        turnContinuationStatus = false;
        selectionContinuationStatus = false;
        bustPending = false;
        hasRolledThisTurn = false;
        bustBonusUsedThisTurn = false;
        rollCountThisTurn = 0;
    }
}

/**
 * @brief Sets the score limit for all players.
 *
 * @param limit The score limit to set.
 */
void GameManager::setScoreLimit(const uint32_t limit)
{
    ruleConfig.clampOpeningScoreLimitTo(limit);
    for (Player& player : players)
        player.getScore().setScoreLimit(limit);
}

/**
 * @brief Starts a turn for the specified player index.
 *
 * @param playerIndex The index of the player to start the turn for.
 */
void GameManager::startTurn(const size_t playerIndex)
{
    clearComputerIntent();
    // Ensure there are players to start a turn.
    if (players.empty()) return;
    if (!ruleConfig.hasScoringRuleEnabled())
        throw std::logic_error("At least one scoring rule must be enabled before starting a turn.");

    // Set current player based on index.
    currentIndex = playerIndex % players.size();
    currentPlayer = &players.at(currentIndex);

    // A steal offer belongs only to the immediately following player.
    if (!ruleConfig.stealingEnabled() || !stealOffer || stealOffer->recipientIndex != currentIndex)
        stealOffer.reset();
    stealContinuationActive = false;

    currentPlayer->getScore().setRoundScore(0);
    currentPlayer->getDice().diceSetMap.clear();
    currentPlayer->getDice().calculateMultipleAvailability();
    manageDiceCount(FULL_SET_OF_DICE);
    initializeRollCycle(true);
}

/**
 * @brief Registers that the current player has rolled this turn.
 */
void GameManager::registerRoll()
{
    clearComputerIntent();
    hasRolledThisTurn = true;
    ++rollCountThisTurn;
}

/**
 * @brief Resets the saved multiple value and updates the current player's score accordingly.
 */
void GameManager::resetSavedMultiple()
{
    valueOfChosenMultiple = 0;
    if (currentPlayer) {
        currentPlayer->getScore().setScoreFromMultiples(0);
    }
}

/**
 * @brief Starts the final round of the game.
 */
void GameManager::startFinalRound()
{
    finalRoundActive = true;
    finalRoundStarterIndex = currentIndex;
    finalRoundLeaderIndex = currentIndex;
}

void GameManager::updateFinalRoundLeaderForCurrentPlayer()
{
    if (!finalRoundActive || players.empty() || currentIndex >= players.size() || finalRoundLeaderIndex >= players.size())
        return;

    const uint32_t currentScore = players[currentIndex].getScore().getPermanentScore();
    const uint32_t leaderScore = players[finalRoundLeaderIndex].getScore().getPermanentScore();
    if (currentScore > leaderScore)
        finalRoundLeaderIndex = currentIndex;
}

/**
 * @brief Checks if the game will end after the current turn.
 *
 * @param playerCount The total number of players in the game.
 *
 * @return True if the game will end after the current turn, false otherwise.
 */
bool GameManager::willEndAfterCurrentTurn(const std::size_t playerCount) const
{
    if (!finalRoundActive || playerCount == 0) return false;
    const std::size_t nextIndex = (currentIndex + 1) % playerCount;
    return nextIndex == finalRoundStarterIndex;
}

/**
 * @brief Resets the final round state.
 */
void GameManager::resetFinalRoundState()
{
    finalRoundActive = false;
    finalRoundStarterIndex = 0;
    finalRoundLeaderIndex = 0;
}

/**
 * @brief Ends the current player's turn and banks their score.
 */
void GameManager::endTurnAndBank()
{
    // Ensure there is a current player.
    if (!canBankCurrentScore())
        return;

    clearComputerIntent();
    // Capture a stealable continuation before clearing the completed turn.
    bustPending = false;
    Score& score = currentPlayer->getScore();
    const uint32_t bankedRoundScore = score.getRoundScore();
    const uint16_t unrolledDice = currentPlayer->getDice().getNumDiceInPlay();

    stealOffer.reset();
    if (ruleConfig.stealingEnabled() && players.size() > 1 &&
        unrolledDice > 0 && unrolledDice < FULL_SET_OF_DICE) {
        stealOffer = StealOffer{
            (currentIndex + 1) % players.size(),
            unrolledDice,
            bankedRoundScore,
            valueOfChosenMultiple,
            score.getScoreFromMultiples()
        };
    }
    stealContinuationActive = false;

    // Bank the current player's round score normally; stealing copies it.
    score.increasePermanentScore(bankedRoundScore);
    score.setRoundScore(0);
    currentPlayer->getDice().setNumDiceInPlay(0);
    turnContinuationStatus = false;
    selectionContinuationStatus = false;
}

/**
 * @brief Checks whether the current player can bank the points accumulated this turn.
 *
 * @return True if the current turn state satisfies the bank requirements.
 */
bool GameManager::canBankCurrentScore() const
{
    if (!currentPlayer || bustPending || !hasRolledThisTurn || !selectedOptionStatus)
        return false;

    const Score& score = currentPlayer->getScore();
    if (score.getRoundScore() == 0)
        return false;

    return score.getPermanentScore() + score.getRoundScore() >= ruleConfig.getOpeningScoreLimit();
}

/**
 * @brief Checks whether the current player has the immediately preceding player's steal offer.
 */
bool GameManager::hasStealOfferForCurrentPlayer() const
{
    return currentPlayer && stealOffer && stealOffer->recipientIndex == currentIndex;
}

/**
 * @brief Checks whether the current player may accept a steal offer.
 *
 * A player must already be on the board at the configured Opening Score. This
 * prevents inherited points from being used to reach the player's first bank.
 */
bool GameManager::canCurrentPlayerSteal() const
{
    return ruleConfig.stealingEnabled() && hasStealOfferForCurrentPlayer() &&
           currentPlayer->getScore().getPermanentScore() >= ruleConfig.getOpeningScoreLimit();
}

uint16_t GameManager::getStealOfferDiceCount() const
{
    return hasStealOfferForCurrentPlayer() ? stealOffer->diceCount : 0;
}

uint32_t GameManager::getStealOfferScore() const
{
    return hasStealOfferForCurrentPlayer() ? stealOffer->roundScore : 0;
}

/**
 * @brief Accepts the current steal offer and restores its continuation state.
 */
bool GameManager::acceptStealOffer()
{
    if (!canCurrentPlayerSteal())
        return false;

    const StealOffer accepted = *stealOffer;
    stealOffer.reset();

    Score& score = currentPlayer->getScore();
    score.setRoundScore(accepted.roundScore);
    score.setScoreFromMultiples(accepted.savedMultipleScore);
    valueOfChosenMultiple = accepted.savedMultipleValue;

    Dice& dice = currentPlayer->getDice();
    dice.diceSetMap.clear();
    dice.setNumDiceInPlay(accepted.diceCount);
    dice.calculateMultipleAvailability();

    turnContinuationStatus = true;
    selectionContinuationStatus = true;
    selectedOptionStatus = false;
    bustPending = false;
    hasRolledThisTurn = false;
    bustBonusUsedThisTurn = false;
    rollCountThisTurn = 0;
    stealContinuationActive = true;
    return true;
}

/**
 * @brief Declines the current steal offer and prepares a fresh six-die turn.
 */
void GameManager::declineStealOffer()
{
    if (!hasStealOfferForCurrentPlayer())
        return;

    stealOffer.reset();
    stealContinuationActive = false;

    Score& score = currentPlayer->getScore();
    score.setRoundScore(0);
    resetSavedMultiple();

    Dice& dice = currentPlayer->getDice();
    dice.diceSetMap.clear();
    dice.calculateMultipleAvailability();
    dice.setNumDiceInPlay(FULL_SET_OF_DICE);
    initializeRollCycle(true);
}

/**
 * @brief Ends both an active steal continuation and any unaccepted offer.
 */
void GameManager::endStealChain()
{
    stealOffer.reset();
    stealContinuationActive = false;
}

/**
 * @brief Switches to the next player in the turn order.
 */
void GameManager::switchToNextPlayer()
{
    clearComputerIntent();
    // Advance to the next player in a circular manner.
    if (players.empty())
        return;

    // Move to the next player index.
    currentIndex = (currentIndex + 1) % players.size();
    currentPlayer = &players[currentIndex];
}

/**
 * @brief Finds the player with the highest permanent score.
 *
 * @return Pointer to the highest scoring player.
 */
Player* GameManager::findHighestScoringPlayer()
{
    if (players.empty())
        return nullptr;

    // Scan for the player with the greatest permanent score.
    Player* highestScoringPlayer = &players.front();
    for (Player& player : players)
        if (player.getScore().getPermanentScore() > highestScoringPlayer->getScore().getPermanentScore())
            highestScoringPlayer = &player;

    // Return pointer to the highest scoring player.
    return highestScoringPlayer;
}

/**
 * @brief Retrieves the player with the highest permanent score.
 *
 * @return Pointer to the winning player, or nullptr if no players exist.
 */
const Player* GameManager::getWinner() const
{
    // Return nullptr if there are no players.
    if (players.empty())
        return nullptr;

    // Find the player with the maximum permanent score.
    const auto iter = std::ranges::max_element(
        players,
        [](const Player& a, const Player& b)->bool {
            return a.getScore().getPermanentScore() < b.getScore().getPermanentScore();
        }
    );

    // Return pointer to the winning player.
    return iter == players.end() ? nullptr : &*iter;
}

/**
 * @brief Gets the value of the chosen multiple for scoring.
 *
 * @return The value of the chosen multiple.
 */
uint16_t GameManager::getValueOfChosenMultiple() const
{
    return valueOfChosenMultiple;
}

/**
 * @brief Sets the value of the chosen multiple for scoring.
 *
 * @param chosenMultipleValue The value to set for the chosen multiple.
 */
void GameManager::setValueOfChosenMultiple(const uint16_t chosenMultipleValue)
{
    valueOfChosenMultiple = chosenMultipleValue;
}

/**
 * @brief Gets the status of whether an option has been selected.
 *
 * @return True if an option has been selected, false otherwise.
 */
bool GameManager::getSelectedOptionStatus() const
{
    return selectedOptionStatus;
}

/**
 * @brief Sets the status of whether an option has been selected.
 *
 * @param isOptionSelected True if an option has been selected, false otherwise.
 */
void GameManager::setSelectedOptionStatus(const bool isOptionSelected)
{
    selectedOptionStatus = isOptionSelected;
}

/**
 * @brief Gets the status of whether the turn should continue.
 *
 * @return True if the turn should continue, false otherwise.
 */
bool GameManager::getTurnContinuationStatus() const
{
    return turnContinuationStatus;
}

/**
 * @brief Sets the status of whether the turn should continue.
 *
 * @param continueTurn True if the turn should continue, false otherwise.
 * @param isBust True if the player has busted, false otherwise.
 */
void GameManager::setTurnContinuationStatus(const bool continueTurn, const bool isBust)
{
    if (!continueTurn || isBust)
        clearComputerIntent();
    // Handle bust scenario.
    if (isBust) {
        turnContinuationStatus = continueTurn;
        bustPending = true;
        return;
    }

    // Clear bust status and bank score if threshold met.
    bustPending = false;

    // Set turn continuation status.
    turnContinuationStatus = continueTurn;
}

/**
 * @brief Gets the status of whether selection should continue.
 *
 * @return True if selection should continue, false otherwise.
 */
bool GameManager::getSelectionContinuationStatus() const
{
    return selectionContinuationStatus;
}

/**
 * @brief Sets the status of whether selection should continue.
 *
 * @param continueSelecting True if selection should continue, false otherwise.
 */
void GameManager::setSelectionContinuationStatus(bool continueSelecting)
{
    if (!continueSelecting && !selectedOptionStatus)
        continueSelecting = true;
    selectionContinuationStatus = continueSelecting;
}

/**
 * @brief Retrieves the current player.
 *
 * @return Pointer to the current player.
 */
Player* GameManager::getCurrentPlayer() const
{
    return currentPlayer;
}
