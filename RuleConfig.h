// RuleConfig.h
/**
Author: Jacob Anderson
Class: ECE-6122-A
Last Date Modified: 12/2/2025

Description:
 This file contains the declaration and implementation of the RuleConfig class,
 which manages the configuration of game rules and settings.
*/

#ifndef ZILCH_RULE_CONFIG_H
#define ZILCH_RULE_CONFIG_H

#include <cstdint>

/**
 * @class RuleConfig RuleConfig.h "RuleConfig.h"
 *
 * @brief Manages the configuration of game rules and settings.
 */
class RuleConfig {
public:
	RuleConfig()
		: enableStraits(true), enableSets(true), enableMultiples(true), enableSingles(true),
		  enableAnimations(true), bankThreshold(1000), enableFirstRollBustBonus(true),
		  enableFinalChase(true), allowTies(false), enableStealing(false) {}

	[[nodiscard]] bool straitsEnabled() const { return enableStraits; }
	[[nodiscard]] bool setsEnabled() const { return enableSets; }
	[[nodiscard]] bool multiplesEnabled() const { return enableMultiples; }
	[[nodiscard]] bool singlesEnabled() const { return enableSingles; }
	[[nodiscard]] uint32_t getBankThreshold() const { return bankThreshold; }
	[[nodiscard]] bool animationsEnabled() const { return enableAnimations; }
	[[nodiscard]] bool firstRollBustBonusEnabled() const { return enableFirstRollBustBonus; }
	[[nodiscard]] bool finalChaseEnabled() const { return enableFinalChase; }
	[[nodiscard]] bool tiesAllowed() const { return allowTies; }
	[[nodiscard]] bool stealingEnabled() const { return enableStealing; }

	void toggleStraits() { enableStraits = !enableStraits; }
	void toggleSets() { enableSets = !enableSets; }
	void toggleMultiples() { enableMultiples = !enableMultiples; }
	void toggleSingles() { enableSingles = !enableSingles; }
	void toggleAnimations() { enableAnimations = !enableAnimations; }
	void toggleFirstRollBustBonus() { enableFirstRollBustBonus = !enableFirstRollBustBonus; }
	void toggleFinalChase() { enableFinalChase = !enableFinalChase; }
	void toggleAllowTies() { allowTies = !allowTies; }
	void toggleStealing() { enableStealing = !enableStealing; }
	void adjustBankThreshold(const int32_t delta) {
		const int64_t updated = static_cast<int64_t>(bankThreshold) + delta;
		bankThreshold = static_cast<uint32_t>(updated < 0 ? 0 : updated);
	}

	void setStraitsEnabled(const bool enabled) { enableStraits = enabled; }
	void setSetsEnabled(const bool enabled) { enableSets = enabled; }
	void setMultiplesEnabled(const bool enabled) { enableMultiples = enabled; }
	void setSinglesEnabled(const bool enabled) { enableSingles = enabled; }
	void setBankThreshold(const uint32_t threshold) { bankThreshold = threshold; }
	void setFirstRollBustBonusEnabled(const bool enabled) { enableFirstRollBustBonus = enabled; }
	void setFinalChaseEnabled(const bool enabled) { enableFinalChase = enabled; }
	void setAllowTies(const bool enabled) { allowTies = enabled; }
	void setStealingEnabled(const bool enabled) { enableStealing = enabled; }

private:
	bool enableStraits;
	bool enableSets;
	bool enableMultiples;
	bool enableSingles;
	bool enableAnimations;
	uint32_t bankThreshold;
	bool enableFirstRollBustBonus;
	bool enableFinalChase;
	bool allowTies;
	bool enableStealing;
};

#endif // ZILCH_RULE_CONFIG_H
