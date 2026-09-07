#include "ComputerStrategy.h"

#include "Dice.h"
#include "GameManager.h"
#include "RuleConfig.h"
#include "Score.h"
#include "rules/RuleManager.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <map>
#include <optional>
#include <stdexcept>

namespace zilch::computer {
namespace {

constexpr Policy EASY_POLICY{
    "Easy",
    {0, 600, 600, 600, 600, 600, 600},
    1.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0
};

constexpr Policy MEDIUM_POLICY{
    "Medium",
    {0, 350, 500, 700, 850, 1000, 1150},
    1.0,
    55.0,
    240.0,
    95.0,
    0.08,
    0.10,
    0.25,
    15.0
};

// Refined against incumbent Hard in a 500,000-game mirrored holdout.
constexpr Policy HARD_STANDARD_POLICY{
    "Hard",
    {0, 200, 1021, 1128, 1506, 2130, 5000},
    1.0045,
    36.0805,
    354.561,
    91.9329,
    0.0,
    0.293194,
    0.193316,
    136.066
};

// Separately trained and holdout-tested because Stealing changes dice value.
constexpr Policy HARD_STEALING_POLICY{
    "Hard with Stealing",
    {0, 313, 313, 1106, 1360, 1360, 1376},
    0.88553,
    91.2663,
    229.628,
    94.2546,
    0.0,
    0.187935,
    0.20764,
    -26.2974
};

ComputerDifficulty currentDifficulty(const GameManager& game)
{
    const Player* player = game.getCurrentPlayer();
    return player && player->isComputer() ? player->getComputerDifficulty() : ComputerDifficulty::Medium;
}

const Policy& currentPolicy(const GameManager& game)
{
    return policyForDifficulty(currentDifficulty(game), game.getRuleConfig().stealingEnabled());
}

std::uint16_t consumedDiceCount(const rules::GameOption& option)
{
    std::uint16_t count = 0;
    for (const auto& [value, copies] : option.consumedDice) {
        static_cast<void>(value);
        count = static_cast<std::uint16_t>(count + copies);
    }
    return count;
}

std::uint16_t nextDiceCount(const GameManager& game, const rules::GameOption& option)
{
    const Player* player = game.getCurrentPlayer();
    if (!player)
        return FULL_SET_OF_DICE;

    const std::uint16_t current = player->getDice().getNumDiceInPlay();
    const std::uint16_t consumed = consumedDiceCount(option);
    return consumed >= current ? FULL_SET_OF_DICE : static_cast<std::uint16_t>(current - consumed);
}

double optionUtility(const GameManager& game, const rules::GameOption& option, const Policy& policy)
{
    const std::uint16_t diceAfterOption = nextDiceCount(game, option);
    double utility = policy.scoreWeight * static_cast<double>(option.pointsAwarded) +
                     policy.remainingDiceWeight * static_cast<double>(diceAfterOption);
    if (diceAfterOption == FULL_SET_OF_DICE)
        utility += policy.hotDiceWeight;
    if (option.ruleType == rules::RuleType::Multiple || option.ruleType == rules::RuleType::AddMultiple)
        utility += policy.multipleWeight;
    if (option.ruleType == rules::RuleType::AddMultiple)
        utility += policy.multipleWeight * 0.5;
    return utility;
}

std::uint32_t maxOpponentScore(const GameManager& game)
{
    std::uint32_t maximum = 0;
    const auto& players = game.getPlayers();
    for (std::size_t index = 0; index < players.size(); ++index) {
        if (index == game.getCurrentIndex())
            continue;
        maximum = std::max(maximum, players[index].getScore().getPermanentScore());
    }
    return maximum;
}

bool usesStandardHard(const GameManager& game)
{
    return currentDifficulty(game) == ComputerDifficulty::Hard && !game.getRuleConfig().stealingEnabled();
}

struct GuaranteedCollection {
    std::uint32_t scoreGain{0};
    std::size_t firstOption{0};
};

GuaranteedCollection maximumGuaranteedCollection(const GameManager& source)
{
    const rules::RuleManager scorer(source.getRuleConfig());
    const auto options = scorer.evaluate(source);
    GuaranteedCollection best;
    for (std::size_t index = 0; index < options.size(); ++index) {
        auto branch = source;
        scorer.apply(branch, options[index]);
        const auto gain = options[index].pointsAwarded + maximumGuaranteedCollection(branch).scoreGain;
        if (gain > best.scoreGain)
            best = {gain, index};
    }
    return best;
}

struct RollEnumerationKey {
    std::uint16_t diceCount{0};
    std::uint16_t savedFace{0};
    std::uint32_t savedScore{0};
    std::array<bool, 4> scoringRules{};
    auto operator<=>(const RollEnumerationKey&) const = default;
};

struct RollEnumeration {
    std::uint32_t outcomes{0};
    std::uint32_t busts{0};
    std::uint64_t totalNewScore{0};
};

RollEnumerationKey enumerationKey(const GameManager& game)
{
    const auto& rules = game.getRuleConfig();
    return {game.getCurrentPlayer()->getDice().getNumDiceInPlay(), game.getValueOfChosenMultiple(),
            game.getCurrentPlayer()->getScore().getScoreFromMultiples(),
            {rules.straightEnabled(), rules.threePairsEnabled(), rules.multiplesEnabled(), rules.singlesEnabled()}};
}

RollEnumeration enumerateRoll(const GameManager& source, const bool includeScore)
{
    const auto count = source.getCurrentPlayer()->getDice().getNumDiceInPlay();
    if (count < 1 || count > 6)
        throw std::logic_error("A next-roll estimate requires one through six dice.");
    RollEnumeration result{1, 0, 0};
    for (std::uint16_t die = 0; die < count; ++die)
        result.outcomes *= 6;
    const rules::RuleManager scorer(source.getRuleConfig());
    for (std::uint32_t outcome = 0; outcome < result.outcomes; ++outcome) {
        auto game = source;
        game.getCurrentPlayer()->getScore().setRoundScore(0);
        auto& dice = game.getCurrentPlayer()->getDice();
        dice.diceSetMap.clear();
        auto encoded = outcome;
        for (std::uint16_t die = 0; die < count; ++die) {
            ++dice.diceSetMap[static_cast<std::uint16_t>(encoded % 6 + 1)];
            encoded /= 6;
        }
        dice.calculateMultipleAvailability();
        if (scorer.evaluate(game).empty())
            ++result.busts;
        else if (includeScore)
            result.totalNewScore += maximumGuaranteedCollection(game).scoreGain;
    }
    return result;
}

std::optional<RollEnumeration> enumerateChain(const GameManager& game)
{
    const auto key = enumerationKey(game);
    if (key.diceCount < 1 || key.diceCount > 3 || key.savedFace == 0 || key.savedScore == 0 ||
        !game.getRuleConfig().multiplesEnabled())
        return std::nullopt;
    // Only scoring state enters the cache. Current unclaimed dice, banked
    // totals, opening score, and Final Chase do not alter next-roll scoring.
    thread_local std::map<RollEnumerationKey, RollEnumeration> cache;
    if (const auto found = cache.find(key); found != cache.end())
        return found->second;
    const auto result = enumerateRoll(game, true);
    cache.emplace(key, result);
    return result;
}

double nextRollBustProbability(const GameManager& game)
{
    if (const auto chain = enumerateChain(game))
        return static_cast<double>(chain->busts) / chain->outcomes;
    const auto key = enumerationKey(game);
    thread_local std::map<RollEnumerationKey, double> cache;
    if (const auto found = cache.find(key); found != cache.end())
        return found->second;
    const auto enumeration = enumerateRoll(game, false);
    const auto probability = static_cast<double>(enumeration.busts) / enumeration.outcomes;
    cache.emplace(key, probability);
    return probability;
}

bool isGuaranteedOutrightWin(const GameManager& game)
{
    if (!game.canBankCurrentScore())
        return false;
    const auto& score = game.getCurrentPlayer()->getScore();
    const auto total = static_cast<std::uint64_t>(score.getPermanentScore()) + score.getRoundScore();
    if (game.isFinalRoundActive() && game.willEndAfterCurrentTurn(game.getPlayers().size()))
        return total > maxOpponentScore(game);
    if (!game.getRuleConfig().finalChaseEnabled() || game.getPlayers().size() == 1)
        return total >= score.getScoreLimit();
    return false;
}

bool canSecureWinByCollecting(const GameManager& source)
{
    if (!usesStandardHard(source))
        return false;
    const bool finalTurn = source.isFinalRoundActive() && source.willEndAfterCurrentTurn(source.getPlayers().size());
    const bool immediateWin = !source.getRuleConfig().finalChaseEnabled() || source.getPlayers().size() == 1;
    if (!finalTurn && !immediateWin)
        return false;
    auto game = source;
    const rules::RuleManager scorer(game.getRuleConfig());
    for (;;) {
        const auto best = maximumGuaranteedCollection(game);
        if (best.scoreGain == 0)
            break;
        scorer.apply(game, scorer.evaluate(game)[best.firstOption]);
    }
    return isGuaranteedOutrightWin(game);
}

void prepareSafeFinish(GameManager& game)
{
    if (!game.getComputerBankCommitted() && canSecureWinByCollecting(game)) {
        game.setComputerBankCommitted(true);
        game.setComputerSafeFinishCommitted(true);
    }
}

std::optional<bool> endgameBankDecision(const GameManager& game, const ComputerDifficulty difficulty)
{
    const Player* player = game.getCurrentPlayer();
    if (!player)
        return false;

    const Score& score = player->getScore();
    const std::uint32_t projectedTotal = score.getPermanentScore() + score.getRoundScore();
    const std::uint32_t opponentScore = maxOpponentScore(game);
    const std::uint32_t winningScore = score.getScoreLimit();
    const std::uint16_t remainingDice = player->getDice().getNumDiceInPlay();

    if (game.isFinalRoundActive()) {
        const bool canTie = game.getRuleConfig().tiesAllowed() && projectedTotal >= opponentScore;
        return canTie || projectedTotal > opponentScore;
    }

    if (projectedTotal >= winningScore) {
        if (!game.getRuleConfig().finalChaseEnabled() || game.getPlayers().size() == 1 ||
            difficulty == ComputerDifficulty::Easy) {
            return true;
        }

        const std::uint32_t opponentDistance = opponentScore >= winningScore ? 0 : winningScore - opponentScore;
        const std::uint32_t desiredBuffer = opponentDistance <= 500 ? 1000 : opponentDistance <= 1000 ? 500 : 0;
        if (desiredBuffer == 0 || projectedTotal >= winningScore + desiredBuffer)
            return true;

        const std::uint16_t minimumDiceToPress = difficulty == ComputerDifficulty::Hard ? 3 : 4;
        return remainingDice < minimumDiceToPress;
    }

    if (!game.getRuleConfig().finalChaseEnabled() || difficulty == ComputerDifficulty::Easy)
        return std::nullopt;

    const std::uint32_t distanceToTarget = winningScore - projectedTotal;
    if (distanceToTarget <= 150) {
        const std::int64_t lead = static_cast<std::int64_t>(projectedTotal) - opponentScore;
        if (lead >= 500)
            return true;
        if (remainingDice >= 3)
            return false;
    }

    return std::nullopt;
}

double adjustedBankThreshold(const GameManager& game, const Policy& policy, const bool chainAware = false)
{
    const Player& player = *game.getCurrentPlayer();
    const std::uint16_t dice = player.getDice().getNumDiceInPlay();
    double threshold = policy.bankThresholdByDice.at(std::min<std::size_t>(dice, FULL_SET_OF_DICE));
    if (chainAware) {
        if (const auto chain = enumerateChain(game); chain && chain->busts != 0) {
            // Frozen chain-scoped candidate: weight 1, symmetric blend. The
            // joint search already includes unclaimed points in bank paths,
            // so its chain crossover is E/p, not (E - unclaimed score)/p.
            threshold += static_cast<double>(chain->totalNewScore) / chain->busts - threshold;
        }
    }
    const std::int64_t lead = static_cast<std::int64_t>(player.getScore().getPermanentScore()) - maxOpponentScore(game);

    if (lead > 0)
        threshold -= static_cast<double>(lead) * policy.leadFactor;
    else
        threshold += static_cast<double>(-lead) * policy.trailFactor;

    const std::int64_t projectedTotal = static_cast<std::int64_t>(player.getScore().getPermanentScore()) +
                                        player.getScore().getRoundScore();
    const std::int64_t distanceToWin = static_cast<std::int64_t>(player.getScore().getScoreLimit()) - projectedTotal;
    const std::int64_t closingWindow = std::max<std::int64_t>(0, 1500 - std::max<std::int64_t>(distanceToWin, 0));
    threshold -= static_cast<double>(closingWindow) * policy.closingFactor;

    return std::clamp(threshold, 200.0, static_cast<double>(player.getScore().getScoreLimit()));
}

bool usesJointSelection(const GameManager& game)
{
    if (!usesStandardHard(game))
        return false;
    // A selected hot-dice path remains committed after its chain is cleared.
    if (game.getComputerJointBankDecision().has_value())
        return true;
    if (!game.getRuleConfig().multiplesEnabled())
        return false;
    if (game.getValueOfChosenMultiple() != 0)
        return true;
    const auto options = rules::RuleManager(game.getRuleConfig()).evaluate(game);
    return std::any_of(options.begin(), options.end(), [](const rules::GameOption& option) {
        return option.ruleType == rules::RuleType::Multiple || option.ruleType == rules::RuleType::AddMultiple;
    });
}

void prepareJointSelection(GameManager& game, const bool requireSelection)
{
    struct Plan {
        std::vector<std::size_t> selections;
        bool bank{false};
        double utility{-std::numeric_limits<double>::infinity()};
        std::uint32_t roundScore{0};
        std::uint16_t nextDice{0};
        bool outrightWin{false};
        bool valid{false};
    };
    Plan best;
    const auto consider = [&](Plan candidate) {
        const bool betterScore = candidate.utility > best.utility + 1e-9;
        const bool sameScore = std::abs(candidate.utility - best.utility) <= 1e-9;
        const bool betterTie = sameScore &&
            ((candidate.bank && !best.bank) ||
             (candidate.bank == best.bank &&
              (candidate.roundScore > best.roundScore ||
               (candidate.roundScore == best.roundScore && candidate.nextDice > best.nextDice))));
        if (!best.valid || (candidate.outrightWin && !best.outrightWin) ||
            (candidate.outrightWin == best.outrightWin && (betterScore || betterTie)))
            best = std::move(candidate);
    };
    const rules::RuleManager scorer(game.getRuleConfig());
    std::vector<std::size_t> path;
    std::function<void(const GameManager&)> visit = [&](const GameManager& state) {
        if (!requireSelection || !path.empty()) {
            const bool canBank = state.canBankCurrentScore();
            const auto endgame = canBank ? endgameBankDecision(state, ComputerDifficulty::Hard) : std::nullopt;
            const auto& player = *state.getCurrentPlayer();
            const auto points = player.getScore().getRoundScore();
            const auto dice = player.getDice().getNumDiceInPlay();
            const bool certainWin = isGuaranteedOutrightWin(state);
            if (canBank && (certainWin || endgame != false))
                consider({path, true, static_cast<double>(points), points, dice, certainWin, true});
            if (!certainWin && endgame != true) {
                const auto probability = nextRollBustProbability(state);
                const auto utility = points + probability * (adjustedBankThreshold(state, currentPolicy(state), true) - points);
                consider({path, false, utility, points, dice, false, true});
            }
        }
        const auto options = scorer.evaluate(state);
        for (std::size_t index = 0; index < options.size(); ++index) {
            auto branch = state;
            scorer.apply(branch, options[index]);
            path.push_back(index);
            visit(branch);
            path.pop_back();
        }
    };
    visit(game);
    if (!best.valid)
        throw std::logic_error("Joint planning requires a legal scoring selection or post-selection action.");
    game.setComputerJointPlan(std::move(best.selections), best.bank);
}

}

std::string_view difficultyName(const ComputerDifficulty difficulty)
{
    switch (difficulty) {
        case ComputerDifficulty::Easy: return "Easy";
        case ComputerDifficulty::Medium: return "Medium";
        case ComputerDifficulty::Hard: return "Hard";
    }
    return "Unknown";
}

const Policy& policyForDifficulty(const ComputerDifficulty difficulty, const bool stealingEnabled)
{
    switch (difficulty) {
        case ComputerDifficulty::Easy: return EASY_POLICY;
        case ComputerDifficulty::Medium: return MEDIUM_POLICY;
        case ComputerDifficulty::Hard: return stealingEnabled ? HARD_STEALING_POLICY : HARD_STANDARD_POLICY;
    }
    return MEDIUM_POLICY;
}

std::size_t chooseOption(GameManager& game, const std::vector<rules::GameOption>& options)
{
    if (options.empty())
        throw std::invalid_argument("A computer cannot choose from an empty scoring-option list.");

    if (usesJointSelection(game)) {
        if (!game.getComputerJointBankDecision().has_value())
            prepareJointSelection(game, true);
        auto& selections = game.accessComputerJointSelections();
        if (selections.empty() || selections.front() >= options.size())
            throw std::logic_error("Joint selection has no legal committed scoring option.");
        const auto choice = selections.front();
        selections.erase(selections.begin());
        return choice;
    }
    prepareSafeFinish(game);
    if (usesStandardHard(game) && game.getComputerSafeFinishCommitted())
        return maximumGuaranteedCollection(game).firstOption;

    const ComputerDifficulty difficulty = currentDifficulty(game);
    std::size_t bestIndex = 0;
    if (difficulty != ComputerDifficulty::Hard ||
        (!game.getRuleConfig().stealingEnabled() && game.getComputerBankCommitted())) {
        for (std::size_t index = 1; index < options.size(); ++index) {
            if (options[index].pointsAwarded > options[bestIndex].pointsAwarded)
                bestIndex = index;
        }
        return bestIndex;
    }

    const Policy& policy = currentPolicy(game);
    double bestUtility = optionUtility(game, options.front(), policy);
    for (std::size_t index = 1; index < options.size(); ++index) {
        const double utility = optionUtility(game, options[index], policy);
        if (utility > bestUtility) {
            bestUtility = utility;
            bestIndex = index;
        }
    }
    return bestIndex;
}

PostSelectionDecision decideAfterSelection(
    GameManager& game,
    const std::vector<rules::GameOption>& remainingOptions)
{
    const ComputerDifficulty difficulty = currentDifficulty(game);
    if (usesJointSelection(game)) {
        if (!game.getComputerJointBankDecision().has_value())
            prepareJointSelection(game, false);
        if (!game.getComputerJointSelections().empty())
            return PostSelectionDecision::SelectAgain;
        // Retain the terminal action until the actual roll/bank. The visual
        // UI can enter waitingForRoll before asking for its next paced action.
        return *game.getComputerJointBankDecision() ? PostSelectionDecision::Bank : PostSelectionDecision::Roll;
    }
    prepareSafeFinish(game);
    const bool collectBeforeBank = difficulty == ComputerDifficulty::Hard &&
                                   !game.getRuleConfig().stealingEnabled();
    if (collectBeforeBank && game.getComputerBankCommitted())
        return remainingOptions.empty() ? PostSelectionDecision::Bank : PostSelectionDecision::SelectAgain;
    if (!remainingOptions.empty()) {
        if (difficulty != ComputerDifficulty::Hard)
            return PostSelectionDecision::SelectAgain;

        const Policy& policy = currentPolicy(game);
        double bestContinuation = optionUtility(game, remainingOptions.front(), policy);
        for (std::size_t index = 1; index < remainingOptions.size(); ++index)
            bestContinuation = std::max(bestContinuation, optionUtility(game, remainingOptions[index], policy));

        const Player& player = *game.getCurrentPlayer();
        const double rollUtility = policy.rollBias +
            policy.remainingDiceWeight * static_cast<double>(player.getDice().getNumDiceInPlay());
        if (bestContinuation >= rollUtility)
            return PostSelectionDecision::SelectAgain;
    }

    if (shouldBank(game)) {
        if (collectBeforeBank && !remainingOptions.empty()) {
            // The original plan has finished choosing dice for a possible roll.
            // Claim free points now, without reconsidering a committed bank if
            // collecting the last scoring die returns all six hot dice.
            game.setComputerBankCommitted(true);
            return PostSelectionDecision::SelectAgain;
        }
        return PostSelectionDecision::Bank;
    }
    return PostSelectionDecision::Roll;
}

TurnAction chooseTurnAction(
    GameManager& game,
    const bool waitingForRoll,
    const bool awaitingSelection,
    const std::vector<rules::GameOption>& remainingOptions)
{
    if (game.getBustPending())
        return TurnAction::AcknowledgeBust;

    if (waitingForRoll) {
        if (game.hasStealOfferForCurrentPlayer()) {
            return shouldAcceptSteal(game) ? TurnAction::AcceptStealAndRoll :
                                             TurnAction::DeclineStealAndRoll;
        }
        if (game.getHasRolledThisTurn() && shouldBank(game))
            return TurnAction::Bank;
        return TurnAction::Roll;
    }

    if (awaitingSelection) {
        if (!remainingOptions.empty() && !game.getSelectedOptionStatus())
            return TurnAction::SelectOption;

        switch (decideAfterSelection(game, remainingOptions)) {
            case PostSelectionDecision::SelectAgain:
                return remainingOptions.empty() ? TurnAction::None : TurnAction::SelectOption;
            case PostSelectionDecision::Roll:
                return TurnAction::Roll;
            case PostSelectionDecision::Bank:
                return TurnAction::Bank;
        }
    }

    return TurnAction::None;
}

bool shouldBank(const GameManager& game)
{
    if (!game.canBankCurrentScore())
        return false;

    const ComputerDifficulty difficulty = currentDifficulty(game);
    if (usesStandardHard(game) && game.getComputerJointBankDecision().has_value())
        return game.getComputerJointSelections().empty() && *game.getComputerJointBankDecision();
    if (usesJointSelection(game)) {
        auto plan = game;
        prepareJointSelection(plan, false);
        return plan.getComputerJointSelections().empty() && *plan.getComputerJointBankDecision();
    }
    if (difficulty == ComputerDifficulty::Hard && !game.getRuleConfig().stealingEnabled() &&
        game.getComputerBankCommitted())
        return true;
    if (const std::optional<bool> endgameDecision = endgameBankDecision(game, difficulty); endgameDecision)
        return *endgameDecision;

    const Player& player = *game.getCurrentPlayer();
    return static_cast<double>(player.getScore().getRoundScore()) >= adjustedBankThreshold(game, currentPolicy(game));
}

bool shouldAcceptSteal(const GameManager& game)
{
    if (!game.canCurrentPlayerSteal())
        return false;

    const ComputerDifficulty difficulty = currentDifficulty(game);
    const std::uint32_t inheritedScore = game.getStealOfferScore();
    const std::uint16_t dice = game.getStealOfferDiceCount();
    if (difficulty == ComputerDifficulty::Easy)
        return inheritedScore >= 600;
    if (difficulty == ComputerDifficulty::Medium) {
        const Policy& policy = policyForDifficulty(ComputerDifficulty::Medium, true);
        return static_cast<double>(inheritedScore) >= policy.bankThresholdByDice.at(dice) * 0.7;
    }

    const Policy& policy = policyForDifficulty(ComputerDifficulty::Hard, true);
    const double continuationUtility = policy.scoreWeight * static_cast<double>(inheritedScore) +
        policy.remainingDiceWeight * static_cast<double>(dice) + policy.rollBias;
    const double freshRollUtility = policy.remainingDiceWeight * FULL_SET_OF_DICE;
    return continuationUtility >= freshRollUtility;
}

}
