// GameManager.h
/**
Author: Jacob Anderson
Class: ECE-6122-A
Last Date Modified: 12/2/2025

Description:
 Header for GameManager class that manages game state, players, turns, and rules.
*/

#ifndef ZILCH_GAME_MANAGER_H
#define ZILCH_GAME_MANAGER_H

#include <optional>
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
	// Look-ahead copies own their active player; never alias a live game's vector.
	GameManager(const GameManager& other);
	GameManager& operator=(const GameManager& other);

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
	void setPlayerSetups(const std::vector<PlayerSetup>& playerSetups);
	[[nodiscard]] const std::vector<Player>& getPlayers() const { return players; }
	[[nodiscard]] std::vector<Player>& accessPlayers() { return players; }

	// Score Limit
	void setScoreLimit(uint32_t limit);

	// Turn Management
	void startTurn(size_t playerIndex);
	void endTurnAndBank();
	[[nodiscard]] bool canBankCurrentScore() const;
	[[nodiscard]] bool hasStealOfferForCurrentPlayer() const;
	[[nodiscard]] bool canCurrentPlayerSteal() const;
	[[nodiscard]] uint16_t getStealOfferDiceCount() const;
	[[nodiscard]] uint32_t getStealOfferScore() const;
	[[nodiscard]] bool acceptStealOffer();
	void declineStealOffer();
	[[nodiscard]] bool isStealContinuationActive() const { return stealContinuationActive; }
	void endStealChain();
	void switchToNextPlayer();
	[[nodiscard]] Player* findHighestScoringPlayer();
	[[nodiscard]] const Player* getWinner() const;

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
	// Per-turn computer intent survives scoring hot dice, but never a new roll.
	[[nodiscard]] bool getComputerBankCommitted() const { return computerBankCommitted; }
	void setComputerBankCommitted(const bool committed) { computerBankCommitted = committed; }
	[[nodiscard]] bool getComputerSafeFinishCommitted() const { return computerSafeFinishCommitted; }
	void setComputerSafeFinishCommitted(const bool committed) { computerSafeFinishCommitted = committed; }
	[[nodiscard]] const std::optional<bool>& getComputerJointBankDecision() const { return computerJointBankDecision; }
	[[nodiscard]] const std::vector<std::size_t>& getComputerJointSelections() const { return computerJointSelections; }
	[[nodiscard]] std::vector<std::size_t>& accessComputerJointSelections() { return computerJointSelections; }
	void setComputerJointPlan(std::vector<std::size_t> selections, bool bank);
	void clearComputerIntent();
	[[nodiscard]] bool getBustBonusUsedThisTurn() const { return bustBonusUsedThisTurn; }
	void setBustBonusUsedThisTurn(const bool used) { bustBonusUsedThisTurn = used; }
	void resetSavedMultiple();
	[[nodiscard]] bool isFinalRoundActive() const { return finalRoundActive; }
	[[nodiscard]] size_t getFinalRoundStarterIndex() const { return finalRoundStarterIndex; }
	[[nodiscard]] size_t getFinalRoundLeaderIndex() const { return finalRoundLeaderIndex; }
	void startFinalRound();
	void updateFinalRoundLeaderForCurrentPlayer();
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
	struct StealOffer {
		size_t recipientIndex;
		uint16_t diceCount;
		uint32_t roundScore;
		uint16_t savedMultipleValue;
		uint32_t savedMultipleScore;
	};

	std::vector<Player> players;
	Player* currentPlayer;
	size_t currentIndex;
	uint16_t valueOfChosenMultiple;
	bool selectedOptionStatus;
	bool turnContinuationStatus;
	bool selectionContinuationStatus;
	bool bustPending{false};
	bool computerBankCommitted{false};
	bool computerSafeFinishCommitted{false};
	std::optional<bool> computerJointBankDecision;
	std::vector<std::size_t> computerJointSelections;
	bool hasRolledThisTurn{false};
	bool bustBonusUsedThisTurn{false};
	uint32_t rollCountThisTurn{0};
	bool finalRoundActive{false};
	size_t finalRoundStarterIndex{0};
	size_t finalRoundLeaderIndex{0};
	std::optional<StealOffer> stealOffer;
	bool stealContinuationActive{false};
	RuleConfig ruleConfig;
	GameStateMachine stateMachine;
};


#endif //ZILCH_GAME_MANAGER_H
