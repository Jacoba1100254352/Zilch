#include "ComputerStrategy.h"
#include "GameManager.h"
#include "rules/RuleManager.h"

#include <array>
#include <charconv>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

struct Input {
    std::uint32_t turnScore{};
    std::uint32_t ownScore{};
    std::uint32_t opponentScore{};
    std::uint32_t target{};
    std::uint32_t opening{};
    bool stealing{};
    bool finalChase{};
    bool finalChaseActive{};
    bool ties{};
    bool sets{};
    std::vector<std::uint32_t> dice;
    std::vector<std::uint32_t> chainScores;
};

std::uint32_t number(const std::string& token)
{
    std::uint32_t value{};
    const auto result = std::from_chars(token.data(), token.data() + token.size(), value);
    if (result.ec != std::errc{} || result.ptr != token.data() + token.size() || value > 1'000'000)
        throw std::invalid_argument("Expected an integer from 0 through 1000000.");
    return value;
}

bool boolean(const std::string& token)
{
    if (token == "1" || token == "true") return true;
    if (token == "0" || token == "false") return false;
    throw std::invalid_argument("Expected 0/1 or false/true.");
}

std::vector<std::uint32_t> numbers(const std::string& token)
{
    std::vector<std::uint32_t> values;
    std::size_t offset = 0;
    for (;;) {
        const auto comma = token.find(',', offset);
        values.push_back(number(token.substr(offset, comma == std::string::npos ? comma : comma - offset)));
        if (comma == std::string::npos) return values;
        offset = comma + 1;
    }
}

Input parse(const std::string& line)
{
    std::istringstream stream(line);
    std::array<std::string, 12> fields;
    for (auto& field : fields) {
        if (!(stream >> field))
            throw std::invalid_argument("Expected exactly 12 whitespace-separated fields.");
    }
    std::string extra;
    if (stream >> extra)
        throw std::invalid_argument("Expected exactly 12 whitespace-separated fields.");
    Input input{number(fields[0]), number(fields[1]), number(fields[2]), number(fields[3]), number(fields[4]),
                boolean(fields[5]), boolean(fields[6]), boolean(fields[7]), boolean(fields[8]), boolean(fields[9]),
                numbers(fields[10]), numbers(fields[11])};
    if (input.target < 1000 || input.opening > input.target)
        throw std::invalid_argument("Target must be at least 1000 and opening must not exceed target.");
    if (input.finalChaseActive && !input.finalChase)
        throw std::invalid_argument("Active Final Chase requires the rule to be enabled.");
    if (input.dice.empty() || input.dice.size() > 6)
        throw std::invalid_argument("Provide between one and six dice.");
    for (const auto die : input.dice) {
        if (die < 1 || die > 6)
            throw std::invalid_argument("Each die value must be from 1 through 6.");
    }
    if (input.chainScores.size() != 6)
        throw std::invalid_argument("Provide six saved multiple scores, ordered by face.");
    unsigned int chains = 0;
    for (const auto score : input.chainScores)
        chains += score != 0;
    if (chains > 1)
        throw std::invalid_argument("This visual game stores one saved multiple; multiple saved chains are not reachable.");
    return input;
}

struct Decision {
    std::array<std::uint32_t, 6> selected{};
    std::string action;
    std::uint32_t scoreGain{};
    std::uint32_t projectedTurnScore{};
    std::uint16_t nextDice{};
    bool canBank{};
};

Decision decide(const Input& input)
{
    GameManager game;
    game.setPlayerSetups({{"Computer", PlayerKind::Computer, ComputerDifficulty::Hard},
                         {"Opponent", PlayerKind::Human, ComputerDifficulty::Medium}});
    game.setScoreLimit(input.target);
    auto& rules = game.accessRuleConfig();
    rules.setOpeningScoreLimit(input.opening);
    rules.setStealingEnabled(input.stealing);
    rules.setFinalChaseEnabled(input.finalChase);
    rules.setAllowTies(input.ties);
    rules.setThreePairsEnabled(input.sets);
    game.accessPlayers()[0].getScore().increasePermanentScore(input.ownScore);
    game.accessPlayers()[1].getScore().increasePermanentScore(input.opponentScore);
    if (input.finalChaseActive) {
        game.startTurn(1);
        game.startFinalRound();
    }
    game.startTurn(0);
    auto& player = *game.getCurrentPlayer();
    player.getScore().setRoundScore(input.turnScore);
    game.manageDiceCount(static_cast<std::uint16_t>(input.dice.size()));
    auto& dice = player.getDice();
    for (const auto die : input.dice)
        ++dice.diceSetMap[static_cast<std::uint16_t>(die)];
    dice.calculateMultipleAvailability();
    for (std::uint16_t face = 1; face <= 6; ++face) {
        if (input.chainScores[face - 1] != 0) {
            game.setValueOfChosenMultiple(face);
            player.getScore().setScoreFromMultiples(input.chainScores[face - 1]);
        }
    }
    game.registerRoll();
    const auto originalDice = dice.diceSetMap;
    const zilch::rules::RuleManager scorer(rules);
    Decision result;
    if (scorer.evaluate(game).empty()) {
        result.action = "Bust";
    } else {
        for (unsigned int step = 0; step <= 6; ++step) {
            const auto options = scorer.evaluate(game);
            // Match the paced visual UI: exhausted options enter waitingForRoll
            // before it asks the action planner for the terminal decision.
            const auto action = zilch::computer::chooseTurnAction(game, options.empty(), !options.empty(), options);
            if (action == zilch::computer::TurnAction::SelectOption) {
                const auto choice = zilch::computer::chooseOption(game, options);
                if (choice >= options.size())
                    throw std::logic_error("Controller returned an invalid option index.");
                scorer.apply(game, options[choice]);
            } else if (action == zilch::computer::TurnAction::Bank || action == zilch::computer::TurnAction::Roll) {
                result.action = action == zilch::computer::TurnAction::Bank ? "Bank" : "Roll";
                break;
            } else {
                throw std::logic_error("Unexpected action at a scoring checkpoint.");
            }
        }
    }
    if (result.action.empty())
        throw std::logic_error("Selection did not terminate within six dice.");
    for (const auto& [face, count] : originalDice) {
        const auto remaining = dice.diceSetMap.find(face);
        result.selected[face - 1] = count - (remaining == dice.diceSetMap.end() ? 0 : remaining->second);
    }
    result.projectedTurnScore = player.getScore().getRoundScore();
    result.scoreGain = result.projectedTurnScore - input.turnScore;
    result.nextDice = dice.getNumDiceInPlay();
    result.canBank = game.canBankCurrentScore();
    return result;
}

void print(const Decision& result, const std::size_t line)
{
    std::cout << "{\"line\":" << line << ",\"selected_counts\":[";
    for (std::size_t index = 0; index < result.selected.size(); ++index)
        std::cout << (index == 0 ? "" : ",") << result.selected[index];
    std::cout << "],\"action\":\"" << result.action << "\",\"score_gain\":" << result.scoreGain
              << ",\"projected_turn_score\":" << result.projectedTurnScore << ",\"next_dice\":" << result.nextDice
              << ",\"can_bank\":" << (result.canBank ? "true" : "false") << "}\n";
}

void selfTest()
{
    const auto check = [](const bool condition, const char* message) {
        if (!condition) throw std::runtime_error(message);
    };
    const auto triple = decide(parse("0 0 0 5000 1000 0 1 0 1 1 6,6,6,5,2,3 0,0,0,0,0,0"));
    check(triple.action == "Roll" && triple.scoreGain == 600 && triple.nextDice == 3,
          "Triple-six checkpoint must preserve the unrelated five for a three-dice reroll.");
    const auto hot = decide(parse("600 1000 1000 10000 1000 0 1 0 1 1 6,1,5 0,0,0,0,0,600"));
    check(hot.action == "Roll" && hot.projectedTurnScore == 1350 && hot.nextDice == 6,
          "Extension and singles must complete the committed hot-dice roll.");
    const auto extension = decide(parse("600 0 0 5000 1000 0 1 0 1 1 6,6,6 0,0,0,0,0,600"));
    check(extension.scoreGain == 4200 && extension.nextDice == 6,
          "Three additional sixes must extend the existing chain.");
    const auto finish = decide(parse("0 4900 5500 5000 1000 0 1 1 0 1 6,6,6,5,2,3 0,0,0,0,0,0"));
    check(finish.action == "Bank" && finish.scoreGain == 650,
          "The real visual action planner must collect an outright final win.");
    const auto noSets = decide(parse("1800 0 0 5000 1000 0 1 0 1 0 2,2,3,3,4,4 0,0,0,0,0,0"));
    check(noSets.action == "Bust", "Disabling Sets must remove the three-pairs option.");
    std::cout << "Visual decision-probe self-test passed.\n";
}

} // namespace

int main(const int argc, const char* const* argv)
{
    try {
        if (argc == 2 && std::string(argv[1]) == "--self-test") {
            selfTest();
            return 0;
        }
        if (argc != 1)
            throw std::invalid_argument("Use no options for JSON-lines input, or --self-test.");
        std::string line;
        std::size_t lineNumber = 0;
        while (std::getline(std::cin, line)) {
            ++lineNumber;
            try {
                print(decide(parse(line)), lineNumber);
            } catch (const std::exception& error) {
                std::cerr << "Input line " << lineNumber << ": " << error.what() << '\n';
                return 1;
            }
        }
        if (!std::cin.eof()) throw std::runtime_error("Unable to read the complete input stream.");
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Visual decision probe: " << error.what() << '\n';
        return 1;
    }
}
