// GameManager.h
/**
Author: Jacob Anderson
Class: ECE-6122-A
Last Date Modified: 12/2/2025

Description:
 Header for GameManager class that manages game state, players, turns, and rules.
*/

#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include "GameStateMachine.h"
#include "Player.h"
#include "RuleConfig.h"

/**
 * @class GameManager GameManager.h "GameManager"
 *
 * @brief Manages the overall game state, players, turns, and rules.
 */
class GameManager {
public:
	// Constructor initializes member variables.
	GameManager() : players({}), currentPlayer(nullptr), currentIndex(0), valueOfChosenMultiple(0), selectedOptionStatus(false),
					turnContinuationStatus(true), selectionContinuationStatus(true) {}

	// Roll Cycle Management
	void initializeRollCycle(bool resetTurnFlags);
	void manageDiceCount(uint16_t numOfDice);

	// RuleConfig
	[[nodiscard]] const RuleConfig& getRuleConfig() const { return ruleConfig; }
	[[nodiscard]] RuleConfig& accessRuleConfig() { return ruleConfig; }
	[[nodiscard]] GameStateMachine& getStateMachine() { return stateMachine; }
	[[nodiscard]] const GameStateMachine& getStateMachine() const { return stateMachine; }

	// Players
	void setPlayers(const std::vector<std::string>& playerNames);
	[[nodiscard]] const std::vector<Player>& getPlayers() const { return players; }
	[[nodiscard]] std::vector<Player>& accessPlayers() { return players; }

	// Score Limit
	void setScoreLimit(uint32_t limit);

	// Turn Management
	void startTurn(size_t playerIndex);
	void endTurnAndBank();
	[[nodiscard]] bool canBankCurrentScore() const;
	void switchToNextPlayer();
	[[nodiscard]] Player* findHighestScoringPlayer();
	[[nodiscard]] Player* getWinner() const;

	// SelectedOptionStatus
	[[nodiscard]] bool getSelectedOptionStatus() const;
	void setSelectedOptionStatus(bool isOptionSelected);

	// TurnContinuationStatus
	[[nodiscard]] bool getTurnContinuationStatus() const;
	[[nodiscard]] bool getBustPending() const { return bustPending; }
	void clearBustPending() { bustPending = false; }
	void setTurnContinuationStatus(bool continueTurn, bool isBust = false);
	[[nodiscard]] uint32_t getRollCountThisTurn() const { return rollCountThisTurn; }
	void registerRoll();
	[[nodiscard]] bool getBustBonusUsedThisTurn() const { return bustBonusUsedThisTurn; }
	void setBustBonusUsedThisTurn(const bool used) { bustBonusUsedThisTurn = used; }
	void resetSavedMultiple();
	[[nodiscard]] bool isFinalRoundActive() const { return finalRoundActive; }
	[[nodiscard]] size_t getFinalRoundStarterIndex() const { return finalRoundStarterIndex; }
	void startFinalRound();
	[[nodiscard]] bool willEndAfterCurrentTurn(std::size_t playerCount) const;
	void resetFinalRoundState();

	// Roll state
	[[nodiscard]] bool getHasRolledThisTurn() const { return hasRolledThisTurn; }
	void setHasRolledThisTurn(const bool rolled) { hasRolledThisTurn = rolled; }

	// SelectionContinuationStatus
	[[nodiscard]] bool getSelectionContinuationStatus() const;
	void setSelectionContinuationStatus(bool continueSelecting);

	// CurrentPlayer
	[[nodiscard]] Player* getCurrentPlayer() const;
	[[nodiscard]] size_t getCurrentIndex() const { return currentIndex; }

	// ValueOfChosenMultiple
	[[nodiscard]] uint16_t getValueOfChosenMultiple() const;
	void setValueOfChosenMultiple(uint16_t chosenMultipleValue);

private:
	std::vector<Player> players;
	Player* currentPlayer;
	size_t currentIndex;
	uint16_t valueOfChosenMultiple;
	bool selectedOptionStatus;
	bool turnContinuationStatus;
	bool selectionContinuationStatus;
	bool bustPending{false};
	bool hasRolledThisTurn{false};
	bool bustBonusUsedThisTurn{false};
	uint32_t rollCountThisTurn{0};
	bool finalRoundActive{false};
	size_t finalRoundStarterIndex{0};
	RuleConfig ruleConfig;
	GameStateMachine stateMachine;
};

#endif // GAMEMANAGER_H
