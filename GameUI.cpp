// GameUI.cpp
/**
Author: Jacob Anderson
Class: ECE-6122-A
Last Date Modified: 12/2/2025

Description:
This file contains the implementation of the GameUI class, which manages the user interface
for the game using SFML. It includes layout constants, typography settings, and methods for rendering
various game screens such as the intro, setup, player turns, and game over.
*/

#include "GameUI.h"
#include "Checker.h"
#include "ComputerStrategy.h"
#include "Dice.h"
#include "DiceRenderer.h"
#include "GameManager.h"
#include "Player.h"
#include "ResourcePath.h"
#include "RuleConfig.h"
#include "rules/RuleManager.h"

#include <algorithm>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace {
    // Window dimensions
    constexpr unsigned WINDOW_WIDTH  = 960;
    constexpr unsigned WINDOW_HEIGHT = 960;

    // Base constants
    constexpr float BASE_GAP = 40.f;
    constexpr float BASE_STEP = 60.f;
    constexpr float BASE_CONTENT_Y = 160.f;

    // Button dimensions
    constexpr float BUTTON_WIDTH    = 220.f;
    constexpr float BUTTON_HEIGHT   = 48.f;
    constexpr float PLAYER_Y_OFFSET = 28.f;
    constexpr float UI_CENTER_X = static_cast<float>(WINDOW_WIDTH) * 0.5f;
    constexpr float PRIMARY_GAP = BASE_GAP;

    // Intro layout
    constexpr float INTRO_PANEL_W = 720.f;
    constexpr float INTRO_PANEL_H = 640.f;
    constexpr float INTRO_PANEL_X = (WINDOW_WIDTH - INTRO_PANEL_W) * 0.5f;
    constexpr float INTRO_PANEL_Y = (WINDOW_HEIGHT - INTRO_PANEL_H) * 0.5f;
    constexpr float INTRO_MARGIN_FRAC_X = 0.1f;   // fraction of panel width for left padding
    constexpr float INTRO_TITLE_Y_FRAC  = 0.1f;   // fraction of panel height for title Y
    constexpr float INTRO_DESC_Y_FRAC   = 0.25f;  // fraction of panel height for description Y
    // constexpr float INTRO_START_Y_FRAC  = 0.48f;  // fraction of panel height for start button Y
    constexpr float INTRO_START_LABEL_OFFSET_X  = 16.f;
    constexpr float INTRO_START_LABEL_OFFSET_Y  = 10.f;
    constexpr float PANEL_OUTLINE_THICKNESS     = 2.f;

    // Setup layout
    constexpr float SETUP_GAP       = BASE_GAP;
    constexpr float SETUP_LEFT_X    = UI_CENTER_X - BUTTON_WIDTH - SETUP_GAP * 0.5f;
    constexpr float SETUP_RIGHT_X   = UI_CENTER_X + SETUP_GAP * 0.5f;
    constexpr float SETUP_ROW1_Y    = 350.f;
    constexpr float SETUP_ROW_STEP  = BASE_STEP;
    constexpr float SETUP_GROUP_GAP = BASE_STEP*0.5f;
    constexpr float SETUP_ROW2_Y    = SETUP_ROW1_Y + SETUP_ROW_STEP;
    constexpr float SETUP_ROW3_Y    = SETUP_ROW2_Y + SETUP_ROW_STEP;
    constexpr float SETUP_TOGGLE_START_Y        = SETUP_ROW3_Y + SETUP_ROW_STEP + SETUP_GROUP_GAP;
    constexpr float SETUP_TOGGLE_STEP           = BASE_STEP;
    constexpr float SETUP_START_BTN_OFFSET      = BASE_STEP*0.5f;
    constexpr float SETUP_LABEL_X               = BASE_GAP;
    constexpr float SETUP_LABEL_Y               = 110.f;
    constexpr float SETUP_BANK_LABEL_Y_OFFSET   = BASE_STEP*0.5f;
    constexpr float SETUP_INFO_X                = 600.f;
    constexpr float SETUP_TITLE_X               = BASE_GAP;
    constexpr float SETUP_TITLE_Y               = BASE_GAP;
    constexpr float NAME_LIST_START_Y           = BASE_CONTENT_Y;
    constexpr float NAME_LIST_X                 = BASE_STEP;
    constexpr float PLAYER_TYPE_X               = 340.f;
    constexpr float PLAYER_TYPE_HIT_WIDTH       = 210.f;

    // Turn layout
    constexpr float TURN_BTN_ROLL_X   = UI_CENTER_X - BUTTON_WIDTH - PRIMARY_GAP * 0.5f;
    constexpr float TURN_BTN_ROLL_Y   = BASE_CONTENT_Y;
    constexpr float TURN_NOTICE_Y     = TURN_BTN_ROLL_Y + 20.f;
    constexpr float TURN_BTN_BANK_X   = UI_CENTER_X + PRIMARY_GAP * 0.5f;
    constexpr float TURN_BTN_BANK_Y   = BASE_CONTENT_Y;
    constexpr float TURN_BTN_PASS_X   = TURN_BTN_ROLL_X;
    constexpr float TURN_BTN_PASS_Y   = BASE_CONTENT_Y;
    constexpr float STEAL_NOTICE_Y    = TURN_BTN_ROLL_Y + BUTTON_HEIGHT + 12.f;
    constexpr float FIRST_BUST_NOTICE_Y = TURN_BTN_ROLL_Y + 200.f;
    constexpr float OPTION_START_Y    = BASE_CONTENT_Y + BASE_STEP + BASE_GAP;
    constexpr float OPTION_STEP_Y     = BASE_STEP;
    constexpr float OPTION_WIDTH      = BUTTON_WIDTH + 200.f;
    constexpr float OPTION_X          = UI_CENTER_X - OPTION_WIDTH * 0.5f;
    constexpr float HEADER_X          = BASE_GAP;
    constexpr float HEADER_Y          = 20.f;
    constexpr float SCORE_Y           = BASE_STEP;
    // constexpr float MULTIPLE_Y        = 90.f;
    constexpr float BUST_X_OFFSET     = -30.f;
    constexpr float BUST_Y_OFFSET     = -180.f;
    constexpr float BUST_Y_FRACTION   = 0.65f;
    constexpr float SCORECARD_RIGHT_X = 700.f;
    constexpr float SCORECARD_TOP_Y   = 20.f;
    constexpr float SCORECARD_LINE_STEP = 26.f;
    // constexpr float SCORECARD_NAME_OFFSET = 0.f;
    constexpr float SCORECARD_SCORE_OFFSET = 180.f;

    // Game over layout
    constexpr float GAMEOVER_CLOSE_X = BASE_STEP;
    constexpr float GAMEOVER_CLOSE_Y = 220.f;
    constexpr float GAMEOVER_TITLE_X = BASE_GAP;
    constexpr float GAMEOVER_TITLE_Y = 120.f;

    // Typography
    constexpr unsigned TITLE_FONT_SIZE   = 48;
    constexpr unsigned INTRO_DESC_SIZE   = 20;
    constexpr unsigned INTRO_RULES_SIZE  = 12;
    constexpr unsigned SETUP_TITLE_SIZE  = 42;
    constexpr unsigned LABEL_FONT_SIZE   = 20;
    constexpr unsigned HEADER_FONT_SIZE  = 26;
    constexpr unsigned SCORE_FONT_SIZE   = 20;
    constexpr unsigned SCORECARD_FONT_SIZE  = 16;
    constexpr unsigned BUTTON_FONT_SIZE     = 18;
    constexpr unsigned BUST_FONT_SIZE       = 36;
    constexpr unsigned GAMEOVER_TITLE_SIZE  = 34;
    constexpr unsigned NOTICE_FONT_SIZE     = 24;
    constexpr float BUTTON_OUTLINE_THICKNESS = 1.5f;
    constexpr float BUTTON_LABEL_OFFSET_X = 8.f;
    constexpr float BUTTON_LABEL_OFFSET_Y = 12.f;
    constexpr float SETUP_BUTTON_LABEL_OFFSET_X = 10.f;
    constexpr float SETUP_BUTTON_LABEL_OFFSET_Y = 10.f;
    constexpr float CLOSE_BUTTON_LABEL_OFFSET   = 12.f;
    constexpr std::size_t NAME_MAX_LEN = 20;
    constexpr float NAME_HIT_WIDTH = 260.f;
    constexpr float SCORE_INPUT_HIT_WIDTH = 220.f;
    constexpr float SCORE_INPUT_HIT_HEIGHT = 32.f;
    constexpr float COMPUTER_ACTION_DELAY_SECONDS = 0.55f;
    
    // Bank threshold reference.
    constexpr float INFO_BASE_Y = SCORE_Y;
    constexpr float INFO_STEP = SCORECARD_LINE_STEP;

    // Colors
    #define COLOR_TEXT_PRIMARY sf::Color::White
    const sf::Color COLOR_BG_MAIN       (10, 20, 40);
    const sf::Color COLOR_BG_SETUP      (12, 28, 56);
    const sf::Color COLOR_PANEL         (18, 32, 64);
    const sf::Color COLOR_PANEL_OUT     (120, 180, 255);
    const sf::Color COLOR_BUTTON        (70, 130, 240);
    const sf::Color COLOR_BUTTON_ALT    (80, 140, 240);
    // const sf::Color COLOR_BUTTON_SETUP  (60, 110, 200);
    const sf::Color COLOR_BUTTON_OUT    (200, 220, 255);
    const sf::Color COLOR_TEXT_MUTED    (200, 220, 255);
    const sf::Color COLOR_TEXT_SCORE    (210, 230, 255);
    const sf::Color COLOR_BG_GAMEOVER   (16, 32, 64);
    const sf::Color COLOR_BUTTON_CLOSE  (150, 60, 80);
    const sf::Color COLOR_BUST_TEXT     (255, 120, 120);
    const sf::Color COLOR_TOGGLE_ON     (60, 180, 90);
    const sf::Color COLOR_TOGGLE_OFF    (190, 70, 70);

    // Game Name
    const std::string GAME_NAME = "Zilch";
}

/**
 * @brief Returns the default button color.
 *
 * @return sf::Color The default button color.
 */
sf::Color GameUI::defaultButtonColor() {
    return Button{}.color;
}

/**
 * @brief Returns the default button text color.
 *
 * @return sf::Color The default button text color.
 */
sf::Color GameUI::defaultButtonTextColor() {
    return Button{}.textColor;
}

bool GameUI::computerActionReady(const std::string& actionKey)
{
    if (computerActionKey != actionKey) {
        computerActionKey = actionKey;
        computerActionClock.restart();
        return false;
    }
    if (computerActionClock.getElapsedTime().asSeconds() < COMPUTER_ACTION_DELAY_SECONDS)
        return false;

    computerActionKey.clear();
    return true;
}

/**
 * @brief Constructs a GameUI instance, initializing the SFML window and loading the UI font.
 */
GameUI::GameUI()
{
    // Set up SFML window with OpenGL context settings.
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 4;
    settings.majorVersion = 2;
    settings.minorVersion = 1;

    // Create the window
    window.create(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), GAME_NAME, sf::Style::Close, settings);
    window.setVerticalSyncEnabled(true);

    // Load UI font
    if (!uiFont.loadFromFile(resolveResourcePath("fonts/FreeSans.ttf")))
        throw std::runtime_error("Failed to load UI font from fonts/FreeSans.ttf");
}

/**
 * @brief Main loop to run the game UI, handling different game states and rendering accordingly.
 *
 * @param game Reference to the GameManager instance.
 */
void GameUI::run(GameManager& game)
{
    // Dice renderer for rendering dice during player turns.
    const Checker checker(game);

    // Game setup variables
    std::vector<PlayerSetup> playerSetups = {{"Player 1"}, {"Player 2"}};
    uint32_t scoreLimit = 5000;

    // Start in intro state
    game.getStateMachine().setState(GameStateMachine::State::Intro);

    // State variables for player turns
    bool waitingForRoll = true;
    bool awaitingSelection = false;
    std::vector<Button> optionButtons;

    // Main loop
    while (window.isOpen()) {
        // Event handling
        std::vector<sf::Event> events;
        sf::Event event{};

        // Handle window events
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            events.push_back(event);
        }

        // Clear the window
        window.clear(COLOR_BG_MAIN);

        // Render based on current game state
        switch (game.getStateMachine().getState()) {
            case GameStateMachine::State::Intro:
                renderIntro(game, playerSetups, scoreLimit, waitingForRoll, awaitingSelection, optionButtons, events);
                break;
            case GameStateMachine::State::Rules:
                renderRules(game, events);
                break;
            case GameStateMachine::State::Setup:
                renderSetup(game, playerSetups, scoreLimit, events);
                waitingForRoll = true;
                awaitingSelection = false;
                optionButtons.clear();
                break;
            case GameStateMachine::State::PlayerTurn:
                renderTurn(game, renderer, checker, optionButtons, waitingForRoll, awaitingSelection, events);
                break;
            case GameStateMachine::State::GameOver:
                renderGameOver(game, events);
                break;
            default: break;
        }

        // Display the rendered frame
        window.display();
    }
}


/**
 * @brief Renders the intro screen with a title and a button to open the setup screen.
 *
 * @param game Reference to the GameManager instance.
 * @param playerSetups Vector of player names and controller choices.
 * @param scoreLimit Reference to the score limit.
 * @param waitingForRoll True if waiting for the player to roll dice.
 * @param awaitingSelection True if awaiting player selection.
 * @param optionButtons Vector of option buttons to display.
 * @param events Vector of SFML events to handle user input.
 */
void GameUI::renderIntro(
    GameManager& game,
    std::vector<PlayerSetup>& playerSetups,
    uint32_t& scoreLimit,
    bool& waitingForRoll,
    bool& awaitingSelection,
    std::vector<Button>& optionButtons,
    const std::vector<sf::Event>& events
)
{
    // Layout three buttons vertically.
    constexpr float introButtonsStartY = INTRO_PANEL_Y + INTRO_PANEL_H * 0.45f;
    constexpr float introButtonsStepY = 70.f;
    constexpr float centerX = INTRO_PANEL_X + (INTRO_PANEL_W - BUTTON_WIDTH) * 0.5f;

    // Create buttons
    Button setupBtn = makeButton(
        centerX,
        introButtonsStartY,
        BUTTON_WIDTH,
        BUTTON_HEIGHT,
        "Setup",
        [&game, &waitingForRoll, &awaitingSelection, &optionButtons]()->void {
            waitingForRoll = true;
            awaitingSelection = false;
            optionButtons.clear();
            game.getStateMachine().setState(GameStateMachine::State::Setup);
        },
        COLOR_BUTTON,
        COLOR_TEXT_PRIMARY
    );
    Button rulesBtn = makeButton(
        centerX,
        introButtonsStartY + introButtonsStepY,
        BUTTON_WIDTH,
        BUTTON_HEIGHT,
        "Rules",
        [&game]()->void {
            game.getStateMachine().setState(GameStateMachine::State::Rules);
        },
        COLOR_BUTTON_ALT,
        COLOR_TEXT_PRIMARY
    );
    const bool canStart = game.getRuleConfig().hasScoringRuleEnabled();
    Button startBtn = makeButton(
        centerX,
        introButtonsStartY + introButtonsStepY * 2.f,
        BUTTON_WIDTH,
        BUTTON_HEIGHT,
        canStart ? "Start" : "Start Disabled",
        [&, canStart]()->void {
            if (!canStart)
                return;
            game.accessRuleConfig().clampOpeningScoreLimitTo(scoreLimit);
            game.setPlayerSetups(playerSetups);
            game.setScoreLimit(scoreLimit);
            game.startTurn(0);
            waitingForRoll = true;
            awaitingSelection = false;
            optionButtons.clear();
            game.getStateMachine().setState(GameStateMachine::State::PlayerTurn);
        },
        canStart ? COLOR_BUTTON_ALT : COLOR_TOGGLE_OFF,
        COLOR_TEXT_PRIMARY
    );

    // Draw intro screen
    sf::RectangleShape panel({INTRO_PANEL_W, INTRO_PANEL_H});
    panel.setPosition(INTRO_PANEL_X, INTRO_PANEL_Y);
    panel.setFillColor(COLOR_PANEL);
    panel.setOutlineThickness(PANEL_OUTLINE_THICKNESS);
    panel.setOutlineColor(COLOR_PANEL_OUT);
    window.pushGLStates();
    window.draw(panel);

    // Draw title and description
    sf::Text title(GAME_NAME, uiFont, TITLE_FONT_SIZE);
    title.setFillColor(COLOR_TEXT_PRIMARY);
    constexpr float introTitleX = INTRO_PANEL_X + INTRO_MARGIN_FRAC_X * INTRO_PANEL_W;
    constexpr float introTitleY = INTRO_PANEL_Y + INTRO_TITLE_Y_FRAC * INTRO_PANEL_H;
    title.setPosition(introTitleX, introTitleY);
    window.draw(title);

    // Draw the description
    sf::Text desc("Start a game, adjust setup, or read the rules.", uiFont, INTRO_DESC_SIZE);
    desc.setFillColor(COLOR_TEXT_MUTED);
    desc.setPosition(introTitleX, INTRO_PANEL_Y + INTRO_DESC_Y_FRAC * INTRO_PANEL_H);
    window.draw(desc);
    if (!canStart) {
        sf::Text warning("Start disabled: enable at least one scoring rule in Setup.", uiFont, INTRO_DESC_SIZE);
        warning.setFillColor(COLOR_BUST_TEXT);
        warning.setPosition(introTitleX, startBtn.y + startBtn.h + 16.f);
        window.draw(warning);
    }

    // Draw buttons
    const std::vector<Button> introButtons {setupBtn, rulesBtn, startBtn};
    for (const Button& b : introButtons) {
        sf::RectangleShape buttonShape({b.w, b.h});
        buttonShape.setPosition(b.x, b.y);
        buttonShape.setFillColor(b.color);
        window.draw(buttonShape);

        sf::Text btnLabel(b.text, uiFont, LABEL_FONT_SIZE);
        btnLabel.setFillColor(b.textColor);
        btnLabel.setPosition(b.x + INTRO_START_LABEL_OFFSET_X, b.y + INTRO_START_LABEL_OFFSET_Y);
        window.draw(btnLabel);
    }

    // Handle button click events
    for (const sf::Event& e : events)
        if (e.type == sf::Event::MouseButtonReleased && e.mouseButton.button == sf::Mouse::Left)
            handleButtonClick(static_cast<float>(e.mouseButton.x), static_cast<float>(e.mouseButton.y), introButtons);

    // Restore previous OpenGL state
    window.popGLStates();
}

void GameUI::renderRules(GameManager& game, const std::vector<sf::Event>& events)
{
    constexpr float panelW = INTRO_PANEL_W;
    constexpr float panelH = INTRO_PANEL_H + 140.f;
    constexpr float panelX = (WINDOW_WIDTH - panelW) * 0.5f;
    constexpr float panelY = (WINDOW_HEIGHT - panelH) * 0.5f;

    constexpr float backBtnW = BUTTON_WIDTH;
    constexpr float backBtnH = BUTTON_HEIGHT;
    constexpr float backBtnX = panelX + (panelW - backBtnW) * 0.5f;
    constexpr float backBtnY = panelY + panelH - backBtnH - 20.f;

    // Create Back button
    Button backBtn = makeButton(
        backBtnX,
        backBtnY,
        backBtnW,
        backBtnH,
        "Back",
        [&game]()->void {
            game.getStateMachine().setState(GameStateMachine::State::Intro);
        },
        COLOR_BUTTON_ALT,
        COLOR_TEXT_PRIMARY
    );

    // Handle button click events
    for (const sf::Event& e : events)
        if (e.type == sf::Event::MouseButtonReleased && e.mouseButton.button == sf::Mouse::Left)
            handleButtonClick(static_cast<float>(e.mouseButton.x), static_cast<float>(e.mouseButton.y), {backBtn});

    // Draw rules screen
    window.pushGLStates();
    window.clear(COLOR_BG_SETUP);

    // Draw panel
    sf::RectangleShape panel({panelW, panelH});
    panel.setPosition(panelX, panelY);
    panel.setFillColor(COLOR_PANEL);
    panel.setOutlineThickness(PANEL_OUTLINE_THICKNESS);
    panel.setOutlineColor(COLOR_PANEL_OUT);
    window.draw(panel);

    // Draw how to play title
    sf::Text title("Rules & How to Play", uiFont, SETUP_TITLE_SIZE);
    title.setFillColor(COLOR_TEXT_PRIMARY);
    title.setPosition(panelX + INTRO_MARGIN_FRAC_X * panelW, panelY + 20.f);
    window.draw(title);

    // Draw rules text
    const std::vector<std::string> lines = {
        "Basics:",
        "\t- Your first bank must reach the configured Opening Score.",
        "\t- Straight (1-2-3-4-5-6) and Three Pairs are worth 1000.",
        "Multiples:",
        "\t- Three of a kind scores 100 x the face (1-1-1 is 1000).",
        "\t- Each extra matching die doubles that combo (3-3-3-3 = 600; 1-1-1-1-1 = 4000).",
        "Singles:",
        "\t- Single 1 = 100; single 5 = 50; other singles don't score.",
        "Flow:",
        "\t- Scoring dice are removed; reroll the rest. Hot dice lets you roll all six again.",
        "\t- Bust: no scoring options loses your unbanked points (first-roll bonus gives +50 if enabled).",
        "\t- Bank after at least one scored option and meeting the Opening Score to end your turn.",
        "Stealing (optional):",
        "\t- Once on the board, roll the prior player's unrolled dice to copy and continue that score.",
        "\t- A fresh roll, bust, or hot dice ends the chain; stealing cannot put a player on the board.",
        "Win:",
        "\t- If final-chase is off: first to reach or exceed the winning score wins.",
        "\t- If final-chase is on: reaching the winning score gives everyone else one last turn.",
        "\t- Ties: allowed per toggle."
    };

    // Draw each line of rules
    float textY = panelY + 90.f;
    constexpr float textX = panelX + INTRO_MARGIN_FRAC_X * panelW;
    for (const std::string& line : lines) {
        sf::Text t(line, uiFont, INTRO_RULES_SIZE);
        t.setFillColor(COLOR_TEXT_MUTED);
        t.setPosition(textX, textY);
        window.draw(t);
        textY += 31.f;
    }

    // Draw Back button
    sf::RectangleShape backShape({backBtn.w, backBtn.h});
    backShape.setPosition(backBtn.x, backBtn.y);
    backShape.setFillColor(backBtn.color);
    window.draw(backShape);

    // Draw Back button label
    sf::Text backLabel(backBtn.text, uiFont, LABEL_FONT_SIZE);
    backLabel.setFillColor(backBtn.textColor);
    backLabel.setPosition(backBtn.x + INTRO_START_LABEL_OFFSET_X, backBtn.y + INTRO_START_LABEL_OFFSET_Y);
    window.draw(backLabel);

    // Restore previous OpenGL state
    window.popGLStates();
}

/**
 * @brief Renders the setup screen where players can be added/removed and game rules configured.
 *
 * @param game Reference to the GameManager instance.
 * @param playerSetups Vector of human/computer player setups to be configured.
 * @param scoreLimit Reference to the score limit to be configured.
 * @param events Vector of SFML events to handle user input.
 */
void GameUI::renderSetup(
    GameManager& game, std::vector<PlayerSetup>& playerSetups, uint32_t& scoreLimit,
    const std::vector<sf::Event>& events
)
{
    // Access rule configuration
    RuleConfig& config = game.accessRuleConfig();

    // Clear name focus if list shrinks.
    if (focusedNameIndex >= static_cast<int>(playerSetups.size())) {
        focusedNameIndex = -1;
        nameEditBuffer.clear();
    }
    
    // Helpers to commit numeric edits.
    auto parseUintSafe = [](const std::string& s, const uint32_t fallback)->uint32_t {
        // Empty string case
        if (s.empty()) return fallback;

        // Try to parse the string as an unsigned long long
        try {
            const uint64_t v = std::stoull(s);
            return static_cast<uint32_t>(std::min<uint64_t>(v, std::numeric_limits<uint32_t>::max()));
        } catch (...) {
            return fallback;
        }
    };
    auto commitScoreLimit = [&]()->void {
        // Commit score limit edit if in progress
        if (editingScoreLimit) {
            const uint32_t parsed = parseUintSafe(scoreLimitBuffer, scoreLimit);
            const uint32_t clamped = std::clamp<uint32_t>(parsed, 1000u, 1000000u);
            scoreLimit = clamped;
            config.clampOpeningScoreLimitTo(scoreLimit);
            editingScoreLimit = false;
            scoreLimitBuffer.clear();
        }
    };
    auto commitBankThreshold = [&]()->void {
        // Commit Opening Score edit if in progress.
        if (editingBankThreshold) {
            const uint32_t parsed = parseUintSafe(bankThresholdBuffer, config.getOpeningScoreLimit());
            const uint32_t clamped = std::clamp<uint32_t>(parsed, 0u, scoreLimit);
            config.setOpeningScoreLimit(clamped);
            editingBankThreshold = false;
            bankThresholdBuffer.clear();
        }
    };

    auto cyclePlayerType = [](PlayerSetup& setup)->void {
        if (setup.kind == PlayerKind::Human) {
            setup.kind = PlayerKind::Computer;
            setup.difficulty = ComputerDifficulty::Easy;
        } else if (setup.difficulty == ComputerDifficulty::Easy) {
            setup.difficulty = ComputerDifficulty::Medium;
        } else if (setup.difficulty == ComputerDifficulty::Medium) {
            setup.difficulty = ComputerDifficulty::Hard;
        } else {
            setup.kind = PlayerKind::Human;
            setup.difficulty = ComputerDifficulty::Medium;
        }
    };

    // Buttons for player management and rule configuration
    std::vector<Button> buttons;

    // Player management buttons
    buttons.emplace_back(makeButton(SETUP_LEFT_X, SETUP_ROW1_Y, BUTTON_WIDTH, BUTTON_HEIGHT, "Add Player", [&playerSetups]()->void {
        if (playerSetups.size() < 6)
            playerSetups.push_back({"Player " + std::to_string(playerSetups.size() + 1)});
    }));
    buttons.emplace_back(makeButton(SETUP_RIGHT_X, SETUP_ROW1_Y, BUTTON_WIDTH, BUTTON_HEIGHT, "Remove Player", [&playerSetups]()->void {
        if (playerSetups.size() > 1)
            playerSetups.pop_back();
    }));
    buttons.emplace_back(makeButton(SETUP_LEFT_X, SETUP_ROW2_Y, BUTTON_WIDTH, BUTTON_HEIGHT, "+500 Winning Score", [&scoreLimit]()->void {
        scoreLimit = std::min<uint32_t>(scoreLimit + 500, 1000000u);
    }));
    buttons.emplace_back(makeButton(SETUP_RIGHT_X, SETUP_ROW2_Y, BUTTON_WIDTH, BUTTON_HEIGHT, "-500 Winning Score", [&scoreLimit, &config]()->void {
        if (scoreLimit > 1000)
            scoreLimit = scoreLimit > 1500 ? scoreLimit - 500 : 1000;
        config.clampOpeningScoreLimitTo(scoreLimit);
    }));
    buttons.emplace_back(makeButton(SETUP_LEFT_X, SETUP_ROW3_Y, BUTTON_WIDTH, BUTTON_HEIGHT, "+100 Opening Score", [&config, &scoreLimit]()->void {
        config.clampOpeningScoreLimitTo(scoreLimit);
        const uint32_t current = config.getOpeningScoreLimit();
        const uint32_t increase = std::min<uint32_t>(100, scoreLimit - current);
        config.adjustOpeningScoreLimit(static_cast<int32_t>(increase));
    }));
    buttons.emplace_back(makeButton(SETUP_RIGHT_X, SETUP_ROW3_Y, BUTTON_WIDTH, BUTTON_HEIGHT, "-100 Opening Score", [&config]()->void {
        config.adjustOpeningScoreLimit(-100);
    }));

    // Rule configuration toggles
    struct ToggleOpt {
        std::string label;
        bool enabled;
        std::function<void()> toggle;
    };
    std::vector<ToggleOpt> toggleOpts = {
        {"Straight",    config.straightEnabled(),   [&config]()->void { config.toggleStraight(); }},
        {"Three Pairs", config.threePairsEnabled(), [&config]()->void { config.toggleThreePairs(); }},
        {"Multiples",  config.multiplesEnabled(),  [&config]()->void { config.toggleMultiples(); }},
        {"Singles",    config.singlesEnabled(),    [&config]()->void { config.toggleSingles(); }},
        {"Animations", config.animationsEnabled(), [&config]()->void { config.toggleAnimations(); }},
        {"First-Roll Bust Bonus", config.firstRollBustBonusEnabled(), [&config]()->void { config.toggleFirstRollBustBonus(); }},
        {"Final Chase Round", config.finalChaseEnabled(), [&config]()->void { config.toggleFinalChase(); }},
        {"Allow Ties", config.tiesAllowed(), [&config]()->void { config.toggleAllowTies(); }},
        {"Stealing", config.stealingEnabled(), [&config]()->void { config.toggleStealing(); }}
    };
    
    // Two-column layout for toggles.
    constexpr float toggleColWidth = BUTTON_WIDTH;
    constexpr float toggleLeftX = UI_CENTER_X - toggleColWidth - SETUP_GAP * 0.5f;
    constexpr float toggleRightX = UI_CENTER_X + SETUP_GAP * 0.5f;
    for (std::size_t i = 0; i < toggleOpts.size(); ++i) {
        const std::size_t col = i % 2;
        const std::size_t row = i / 2;
        const float y = SETUP_TOGGLE_START_Y + static_cast<float>(row) * SETUP_TOGGLE_STEP;
        const float x = (col == 0) ? toggleLeftX : toggleRightX;
        const auto& [label, enabled, toggle] = toggleOpts[i];
        const sf::Color toggleColor = enabled ? COLOR_TOGGLE_ON : COLOR_TOGGLE_OFF;
        buttons.emplace_back(makeButton(
            x,
            y,
            toggleColWidth,
            BUTTON_HEIGHT,
            label + (enabled ? " (ON)" : " (OFF)"),
            toggle,
            toggleColor,
            COLOR_TEXT_PRIMARY
        ));
    }

    // Back to intro button
    const std::size_t toggleRows = (toggleOpts.size() + 1) / 2;
    const float backBtnY = SETUP_TOGGLE_START_Y + static_cast<float>(toggleRows) * SETUP_TOGGLE_STEP + SETUP_START_BTN_OFFSET;
    buttons.emplace_back(makeButton(SETUP_LEFT_X, backBtnY, BUTTON_WIDTH + 140.f, BUTTON_HEIGHT, "Back", [&]()->void {
        game.getStateMachine().setState(GameStateMachine::State::Intro);
    }));

    // Process setup events: name input + buttons.
    for (const sf::Event& e : events) {
        if (e.type == sf::Event::TextEntered) {
            const sf::Uint32 ASCII = e.text.unicode;
            // Name editing
            if (focusedNameIndex >= 0) {
                if (ASCII >= 32 && ASCII < 127 && nameEditBuffer.size() < NAME_MAX_LEN) {
                    nameEditBuffer.push_back(static_cast<char>(ASCII));
                    playerSetups[static_cast<std::size_t>(focusedNameIndex)].name = nameEditBuffer;
                } else if (ASCII == 8) { // backspace
                    if (!nameEditBuffer.empty()) {
                        nameEditBuffer.pop_back();
                        playerSetups[static_cast<std::size_t>(focusedNameIndex)].name = nameEditBuffer;
                    }
                } else if (ASCII == 13 || ASCII == 27) { // enter or escape
                    if (ASCII == 13 && focusedNameIndex == static_cast<int>(playerSetups.size() - 1) && playerSetups.size() < 6) {
                        // Add a new player and focus it.
                        const std::string newName = "Player " + std::to_string(playerSetups.size() + 1);
                        playerSetups.push_back({newName});
                        focusedNameIndex = static_cast<int>(playerSetups.size() - 1);
                        nameEditBuffer = newName;
                    } else {
                        focusedNameIndex = -1;
                        nameEditBuffer.clear();
                    }
                }
            } else if (editingScoreLimit) {
                if (ASCII >= '0' && ASCII <= '9' && scoreLimitBuffer.size() < 9) {
                    scoreLimitBuffer.push_back(static_cast<char>(ASCII));
                } else if (ASCII == 8) {
                    if (!scoreLimitBuffer.empty()) scoreLimitBuffer.pop_back();
                } else if (ASCII == 13 || ASCII == 27) {
                    commitScoreLimit();
                }
            } else if (editingBankThreshold) {
                if (ASCII >= '0' && ASCII <= '9' && bankThresholdBuffer.size() < 9) {
                    bankThresholdBuffer.push_back(static_cast<char>(ASCII));
                } else if (ASCII == 8) {
                    if (!bankThresholdBuffer.empty()) bankThresholdBuffer.pop_back();
                } else if (ASCII == 13 || ASCII == 27) {
                    commitBankThreshold();
                }
            }
        } else if (e.type == sf::Event::MouseButtonReleased && e.mouseButton.button == sf::Mouse::Left) {
            // Get mouse position
            const float mx = static_cast<float>(e.mouseButton.x);
            const float my = static_cast<float>(e.mouseButton.y);

            // Check score limit hit
            constexpr float scoreX = SETUP_INFO_X;
            constexpr float scoreY = SETUP_LABEL_Y;
            constexpr float bankY = SETUP_LABEL_Y + SETUP_BANK_LABEL_Y_OFFSET;

            // Score limit and Opening Score hit test.
            bool consumed = false;
            if (mx >= scoreX && mx <= scoreX + SCORE_INPUT_HIT_WIDTH && my >= scoreY && my <= scoreY + SCORE_INPUT_HIT_HEIGHT) {
                commitBankThreshold();
                commitScoreLimit();
                focusedNameIndex = -1;
                nameEditBuffer.clear();
                editingScoreLimit = true;
                scoreLimitBuffer = std::to_string(scoreLimit);
                consumed = true;
            } else if (mx >= scoreX && mx <= scoreX + SCORE_INPUT_HIT_WIDTH && my >= bankY && my <= bankY + SCORE_INPUT_HIT_HEIGHT) {
                commitScoreLimit();
                commitBankThreshold();
                focusedNameIndex = -1;
                nameEditBuffer.clear();
                editingBankThreshold = true;
                bankThresholdBuffer = std::to_string(config.getOpeningScoreLimit());
                consumed = true;
            }

            // Player-type hit test. Each click cycles Human, Easy, Medium, Hard.
            if (!consumed) {
                float hitY = NAME_LIST_START_Y;
                for (PlayerSetup& setup : playerSetups) {
                    if (mx >= PLAYER_TYPE_X && mx <= PLAYER_TYPE_X + PLAYER_TYPE_HIT_WIDTH &&
                        my >= hitY && my <= hitY + PLAYER_Y_OFFSET) {
                        commitScoreLimit();
                        commitBankThreshold();
                        focusedNameIndex = -1;
                        nameEditBuffer.clear();
                        cyclePlayerType(setup);
                        consumed = true;
                        break;
                    }
                    hitY += PLAYER_Y_OFFSET;
                }
            }

            // Name hit test
            if (!consumed) {
                bool hitName = false;
                float hitY = NAME_LIST_START_Y;
                for (std::size_t i = 0; i < playerSetups.size(); ++i) {
                    const float y0 = hitY;
                    const float y1 = hitY + PLAYER_Y_OFFSET;
                    if (mx >= NAME_LIST_X && mx <= NAME_LIST_X + NAME_HIT_WIDTH && my >= y0 && my <= y1) {
                        commitScoreLimit();
                        commitBankThreshold();
                        focusedNameIndex = static_cast<int>(i);
                        nameEditBuffer = playerSetups[i].name;
                        hitName = true;
                        break;
                    }
                    hitY += PLAYER_Y_OFFSET;
                }
                if (!hitName) {
                    commitScoreLimit();
                    commitBankThreshold();
                    focusedNameIndex = -1;
                    nameEditBuffer.clear();
                }
            }

            // Handle button clicks
            handleButtonClick(mx, my, buttons);
        }
    }

    // Begin setup screen rendering.
    window.pushGLStates();
    window.clear(COLOR_BG_SETUP);

    // Display setup title.
    sf::Text title("Setup", uiFont, SETUP_TITLE_SIZE);
    title.setFillColor(COLOR_TEXT_PRIMARY);
    title.setPosition(SETUP_TITLE_X, SETUP_TITLE_Y);
    window.draw(title);

    // Display player label.
    sf::Text playerLabel("Players:", uiFont, LABEL_FONT_SIZE);
    playerLabel.setFillColor(COLOR_TEXT_MUTED);
    playerLabel.setPosition(SETUP_LABEL_X, SETUP_LABEL_Y);
    window.draw(playerLabel);

    sf::Text typeLabel("Controller (click to change):", uiFont, LABEL_FONT_SIZE);
    typeLabel.setFillColor(COLOR_TEXT_MUTED);
    typeLabel.setPosition(PLAYER_TYPE_X, SETUP_LABEL_Y);
    window.draw(typeLabel);

    // List configured player names.
    float nameY = NAME_LIST_START_Y;
    for (std::size_t idx = 0; idx < playerSetups.size(); ++idx) {
        std::string display = playerSetups[idx].name;
        if (static_cast<int>(idx) == focusedNameIndex)
            display += "|";
        sf::Text t(display, uiFont, LABEL_FONT_SIZE);
        t.setFillColor(COLOR_TEXT_PRIMARY);
        t.setPosition(NAME_LIST_X, nameY);
        window.draw(t);

        std::string typeName = "Human";
        if (playerSetups[idx].kind == PlayerKind::Computer)
            typeName = "Computer: " + std::string(zilch::computer::difficultyName(playerSetups[idx].difficulty));
        sf::Text typeText(typeName, uiFont, LABEL_FONT_SIZE);
        typeText.setFillColor(playerSetups[idx].kind == PlayerKind::Computer ? sf::Color(230, 210, 80, 240) : COLOR_TEXT_PRIMARY);
        typeText.setPosition(PLAYER_TYPE_X, nameY);
        window.draw(typeText);

        nameY += PLAYER_Y_OFFSET;
    }

    // Display the configured Winning Score.
    const bool scoreFocused = editingScoreLimit;
    const std::string scoreDisplay = scoreFocused ? scoreLimitBuffer + "|" : std::to_string(scoreLimit);
    sf::Text scoreLabel("Winning Score: " + scoreDisplay, uiFont, LABEL_FONT_SIZE);
    scoreLabel.setFillColor(COLOR_TEXT_PRIMARY);
    scoreLabel.setPosition(SETUP_INFO_X, SETUP_LABEL_Y);
    window.draw(scoreLabel);

    // Display the configured Opening Score.
    const bool bankFocused = editingBankThreshold;
    const std::string bankDisplay = bankFocused ? bankThresholdBuffer + "|" : std::to_string(config.getOpeningScoreLimit());
    sf::Text bankLabel("Opening Score: " + bankDisplay, uiFont, LABEL_FONT_SIZE);
    bankLabel.setFillColor(COLOR_TEXT_PRIMARY);
    bankLabel.setPosition(SETUP_INFO_X, SETUP_LABEL_Y + SETUP_BANK_LABEL_Y_OFFSET);
    window.draw(bankLabel);

    if (!config.hasScoringRuleEnabled()) {
        sf::Text warning("Start disabled: enable at least one scoring rule.", uiFont, INTRO_DESC_SIZE);
        warning.setFillColor(COLOR_BUST_TEXT);
        warning.setPosition(SETUP_LABEL_X, SETUP_ROW1_Y - 28.f);
        window.draw(warning);
    }

    // Draw all setup buttons.
    for (const Button& b : buttons) {
        // Draw button rectangle.
        sf::RectangleShape rect({b.w, b.h});
        rect.setPosition(b.x, b.y);
        rect.setFillColor(b.color);
        rect.setOutlineThickness(BUTTON_OUTLINE_THICKNESS);
        rect.setOutlineColor(COLOR_BUTTON_OUT);
        window.draw(rect);

        // Draw button label.
        sf::Text label(b.text, uiFont, BUTTON_FONT_SIZE);
        label.setFillColor(b.textColor);
        label.setPosition(b.x + SETUP_BUTTON_LABEL_OFFSET_X, b.y + SETUP_BUTTON_LABEL_OFFSET_Y);
        window.draw(label);
    }

    // End setup screen rendering.
    window.popGLStates();
}

/**
 * @brief Renders the main game turn interface, including player info, buttons, and dice.
 *
 * @param game The game manager instance.
 * @param diceRenderer The dice renderer.
 * @param checker The checker for available options.
 * @param optionButtons The vector of option buttons.
 * @param waitingForRoll True if waiting for the player to roll the dice.
 * @param awaitingSelection True if awaiting the player's selection of options.
 * @param events The vector of SFML events to process.
 */
void GameUI::renderTurn(
    GameManager& game, const DiceRenderer& diceRenderer, const Checker& checker, std::vector<Button>& optionButtons,
    bool& waitingForRoll, bool& awaitingSelection, const std::vector<sf::Event>& events
)
{
    // Fetch the active player and bail if unavailable.
    Player* player = game.getCurrentPlayer();
    if (!player) return;

    // Build the set of always-visible primary buttons for this frame.
    std::vector<Button> primaryButtons;
    auto advanceToNextOrEnd = [&]()->void {
        const std::size_t pcount = game.getPlayers().size();
        if (game.getRuleConfig().finalChaseEnabled() && game.isFinalRoundActive() && game.willEndAfterCurrentTurn(pcount)) {
            game.getStateMachine().setState(GameStateMachine::State::GameOver);
        } else {
            game.switchToNextPlayer();
            game.startTurn(game.getCurrentIndex());
            waitingForRoll = true;
            awaitingSelection = false;
            optionButtons.clear();
        }
    };

    // Helper to sync renderer with remaining dice.
    auto refreshDiceView = [&diceRenderer, player]()->void {
        std::vector<uint16_t> remaining;
        const Dice& d = player->getDice();
        for (const auto& [val, count] : d.diceSetMap) {
            for (uint16_t c = 0; c < count; ++c) remaining.push_back(val);
        }
        diceRenderer.updateValues(remaining);
    };

    // Helper to roll immediately (with or without animation) and rebuild options.
    auto performRoll = [&]()->void {
        waitingForRoll = false;
        awaitingSelection = true;
        game.registerRoll();
        game.setSelectedOptionStatus(false);
        player->getDice().rollDice();
        // Reset saved multiple when rolling a fresh hand.
        if (player->getDice().getNumDiceInPlay() == FULL_SET_OF_DICE)
            game.resetSavedMultiple();

        // Update dice renderer with the new roll.
        const Dice diceCopy = player->getDice();
        if (game.getRuleConfig().animationsEnabled()) {
            diceRenderer.startRoll(diceCopy.getLastRoll()); // animated
        } else {
            diceRenderer.updateValues(diceCopy.getLastRoll()); // instant update
        }

        // Rebuild option buttons based on the new roll.
        optionButtons.clear();
        if (const std::vector<Checker::Option> options = checker.getAvailableOptions(); options.empty()) {
            // Bust immediately if a roll has no options.
            checker.handleBusts();
            awaitingSelection = false;
            waitingForRoll = game.getTurnContinuationStatus();
        } else {
            // Populate scoring options after a roll.
            float y = OPTION_START_Y;
            for (const auto& [label, action] : options) {
                // Wrap the action to refresh the displayed dice after selection.
                auto wrapped = [&, action]()->void {
                    action();
                    refreshDiceView();
                };
                optionButtons.emplace_back(makeButton(OPTION_X, y, OPTION_WIDTH, BUTTON_HEIGHT, label, wrapped));
                y += OPTION_STEP_Y;
            }
        }
    };

    auto bankAndAdvance = [&]()->void {
        if (!game.canBankCurrentScore())
            return;

        game.endTurnAndBank();
        const uint32_t newTotal = player->getScore().getPermanentScore();
        const uint32_t scoreLimit = player->getScore().getScoreLimit();

        if (game.getRuleConfig().finalChaseEnabled()) {
            if (!game.isFinalRoundActive() && newTotal >= scoreLimit)
                game.startFinalRound();
            game.updateFinalRoundLeaderForCurrentPlayer();
            advanceToNextOrEnd();
        } else if (newTotal >= scoreLimit) {
            game.getStateMachine().setState(GameStateMachine::State::GameOver);
        } else {
            advanceToNextOrEnd();
        }
    };

    auto acknowledgeBust = [&]()->void {
        game.clearBustPending();
        advanceToNextOrEnd();
    };

    // Offer either the steal decision or the normal roll action.
    if (waitingForRoll && !game.getBustPending()) {
        if (game.hasStealOfferForCurrentPlayer()) {
            if (game.canCurrentPlayerSteal()) {
                const std::string stealLabel = "Steal " + std::to_string(game.getStealOfferDiceCount()) +
                                               " Dice (+" + std::to_string(game.getStealOfferScore()) + ")";
                primaryButtons.emplace_back(makeButton(TURN_BTN_ROLL_X, TURN_BTN_ROLL_Y, BUTTON_WIDTH, BUTTON_HEIGHT, stealLabel, [&]()->void {
                    if (game.acceptStealOffer())
                        performRoll();
                }));
            }

            const float freshButtonX = game.canCurrentPlayerSteal() ? TURN_BTN_BANK_X : TURN_BTN_ROLL_X;
            primaryButtons.emplace_back(makeButton(freshButtonX, TURN_BTN_ROLL_Y, BUTTON_WIDTH, BUTTON_HEIGHT, "Fresh Roll (6 Dice)", [&]()->void {
                game.declineStealOffer();
                performRoll();
            }));
        } else {
            const char* rollLabel = game.getHasRolledThisTurn() ? "Roll Again" : "Roll Dice";
            primaryButtons.emplace_back(makeButton(TURN_BTN_ROLL_X, TURN_BTN_ROLL_Y, BUTTON_WIDTH, BUTTON_HEIGHT, rollLabel, [&]()->void {
                performRoll();
            }));
        }
    }

    // Allow banking after at least one roll if total (permanent + round) meets threshold and no bust is pending.
    if (game.canBankCurrentScore())
        primaryButtons.emplace_back(makeButton(TURN_BTN_BANK_X, TURN_BTN_BANK_Y, BUTTON_WIDTH, BUTTON_HEIGHT, "Bank Score", bankAndAdvance));

    // Offer roll again after at least one option is chosen and dice remain.
    if (awaitingSelection && game.getSelectedOptionStatus() && player->getDice().getNumDiceInPlay() > 0 && !game.getBustPending()) {
        primaryButtons.emplace_back(makeButton(TURN_BTN_ROLL_X, TURN_BTN_ROLL_Y, BUTTON_WIDTH, BUTTON_HEIGHT, "Roll Again", [&]()->void {
            performRoll();
        }));
    }

    // Present bust acknowledgment button when a bust is pending.
    if (game.getBustPending()) {
        primaryButtons.emplace_back(makeButton(TURN_BTN_PASS_X, TURN_BTN_PASS_Y, BUTTON_WIDTH, BUTTON_HEIGHT, "Pass Dice", acknowledgeBust));
    }

    // Automatically prompt for roll if no options are available but the turn continues.
    if (!waitingForRoll && !awaitingSelection && optionButtons.empty() && game.getTurnContinuationStatus()) {
        waitingForRoll = true;
        awaitingSelection = false;
    }

    // Handle button click events for both option and primary buttons.
    for (const sf::Event& e : events)
        if (!player->isComputer() && !renderer.isAnimating() && e.type == sf::Event::MouseButtonReleased && e.mouseButton.button == sf::Mouse::Left) {
            const auto mx = static_cast<float>(e.mouseButton.x);
            const auto my = static_cast<float>(e.mouseButton.y);
            handleButtonClick(mx, my, optionButtons);
            handleButtonClick(mx, my, primaryButtons);
        }

    // Refresh options while we are in a selection state.
    if (awaitingSelection) {
        if (const std::vector<Checker::Option> options = checker.getAvailableOptions(); options.empty()) {
            // If the player already scored something, advance to the roll phase; otherwise bust.
            optionButtons.clear();
            if (game.getSelectedOptionStatus() && player->getDice().getNumDiceInPlay() > 0) {
                awaitingSelection = false;
                waitingForRoll = true;
            } else {
                checker.handleBusts();
                awaitingSelection = false;
                waitingForRoll = game.getTurnContinuationStatus();
                refreshDiceView();
            }
        } else {
            // Rebuild visible options from the latest checker results.
            optionButtons.clear();
            float y = OPTION_START_Y;
            for (const auto& [label, action] : options) {
                // Wrap the action to refresh the displayed dice after selection.
                auto wrapped = [&, action]()->void {
                    action();
                    refreshDiceView();
                };
                optionButtons.emplace_back(makeButton(OPTION_X, y, OPTION_WIDTH, BUTTON_HEIGHT, label, wrapped));
                y += OPTION_STEP_Y;
            }
        }
    }

    // Drive one visible computer action at a time after any dice animation completes.
    if (player->isComputer() && !renderer.isAnimating() &&
        game.getStateMachine().getState() == GameStateMachine::State::PlayerTurn) {
        std::ostringstream actionKey;
        actionKey << game.getCurrentIndex() << ':' << waitingForRoll << ':' << awaitingSelection << ':'
                  << game.getBustPending() << ':' << game.getHasRolledThisTurn() << ':'
                  << game.getSelectedOptionStatus() << ':' << game.getRollCountThisTurn() << ':'
                  << player->getScore().getRoundScore() << ':' << player->getDice().getNumDiceInPlay() << ':'
                  << game.getStealOfferScore() << ':' << game.getStealOfferDiceCount();
        for (const auto& [value, count] : player->getDice().diceSetMap)
            actionKey << ':' << value << 'x' << count;

        if (computerActionReady(actionKey.str())) {
            zilch::rules::RuleManager ruleManager(game.getRuleConfig());
            const std::vector<zilch::rules::GameOption> options =
                awaitingSelection ? ruleManager.evaluate(game) : std::vector<zilch::rules::GameOption>{};
            switch (zilch::computer::chooseTurnAction(game, waitingForRoll, awaitingSelection, options)) {
                case zilch::computer::TurnAction::AcknowledgeBust:
                    acknowledgeBust();
                    break;
                case zilch::computer::TurnAction::AcceptStealAndRoll:
                    if (!game.acceptStealOffer())
                        game.declineStealOffer();
                    performRoll();
                    break;
                case zilch::computer::TurnAction::DeclineStealAndRoll:
                    game.declineStealOffer();
                    performRoll();
                    break;
                case zilch::computer::TurnAction::SelectOption:
                    if (!options.empty()) {
                        ruleManager.apply(game, options[zilch::computer::chooseOption(game, options)]);
                        optionButtons.clear();
                        refreshDiceView();
                    }
                    break;
                case zilch::computer::TurnAction::Roll:
                    performRoll();
                    break;
                case zilch::computer::TurnAction::Bank:
                    bankAndAdvance();
                    break;
                case zilch::computer::TurnAction::None:
                    break;
            }
        }
    } else {
        computerActionKey.clear();
    }

    // If the turn is marked as over and no bust is pending, rotate to the next player.
    if (!game.getTurnContinuationStatus() && !game.getBustPending() && game.getStateMachine().getState() == GameStateMachine::State::PlayerTurn)
        advanceToNextOrEnd();

    // Begin turn rendering.
    window.pushGLStates();
    window.clear(COLOR_BG_MAIN);

    // Render dice.
    const bool animating = renderer.isAnimating();

    // Draw player name.
    const std::string headerText = "Current Player: " + player->getName();
    sf::Text header(headerText, uiFont, HEADER_FONT_SIZE);
    header.setFillColor(COLOR_TEXT_PRIMARY);
    header.setPosition(HEADER_X, HEADER_Y);
    window.draw(header);

    // Draw Opening Score information.
    const std::string thresholdText = "Opening Score: " + std::to_string(game.getRuleConfig().getOpeningScoreLimit());
    sf::Text threshold(thresholdText, uiFont, SCORE_FONT_SIZE);
    threshold.setFillColor(COLOR_TEXT_MUTED);
    threshold.setPosition(HEADER_X, INFO_BASE_Y);
    window.draw(threshold);

    // Draw score summary.
    const std::string scores = "Round: " + std::to_string(player->getScore().getRoundScore()) +
                        "  |  Total: " + std::to_string(player->getScore().getPermanentScore());
    sf::Text scoreText(scores, uiFont, SCORE_FONT_SIZE);
    scoreText.setFillColor(COLOR_TEXT_SCORE);
    scoreText.setPosition(HEADER_X, INFO_BASE_Y + INFO_STEP);
    window.draw(scoreText);

    // Draw saved multiple info.
    const uint16_t savedMult = game.getValueOfChosenMultiple();
    const std::string multLine = "Multiple Saved: " + std::string(savedMult == 0 ? "None" : std::to_string(savedMult));
    sf::Text multText(multLine, uiFont, SCORE_FONT_SIZE);
    multText.setFillColor(COLOR_TEXT_SCORE);
    multText.setPosition(HEADER_X, INFO_BASE_Y + INFO_STEP * 2.f);
    window.draw(multText);

    // Final round notice.
    if (game.getRuleConfig().finalChaseEnabled() && game.isFinalRoundActive()) {
        const Player* starter = nullptr;
        const std::vector<Player>& players = game.getPlayers();
        if (!players.empty() && game.getFinalRoundStarterIndex() < players.size())
            starter = &players[game.getFinalRoundStarterIndex()];
        const uint32_t endLimit = !players.empty() ? players.front().getScore().getScoreLimit() : 0;
        std::string notice = "Final round: ";
        notice += starter ? starter->getName() : "A player";
        notice += " passed ";
        notice += std::to_string(endLimit);
        notice += ". Everyone else gets one last shot.";

        sf::Text finalText(notice, uiFont, NOTICE_FONT_SIZE);
        finalText.setFillColor(sf::Color(230, 210, 80, 240));
        finalText.setPosition(BASE_GAP, static_cast<float>(window.getSize().y) - 80.f);
        window.draw(finalText);
    }

    // Draw bust message when applicable (only when not animating).
    if (game.getBustPending() && !animating) {
        const sf::Vector2u tsize = window.getSize();
        sf::Text bustText("Bust", uiFont, BUST_FONT_SIZE);
        bustText.setFillColor(COLOR_BUST_TEXT);
        bustText.setPosition(static_cast<float>(tsize.x) * 0.5f + BUST_X_OFFSET, static_cast<float>(tsize.y) * BUST_Y_FRACTION + BUST_Y_OFFSET);
        window.draw(bustText);
    }

    // Show rolling indicator while animating.
    if (animating) {
        const sf::Vector2u tsize = window.getSize();
        sf::Text rollingText("Rolling...", uiFont, NOTICE_FONT_SIZE);
        rollingText.setFillColor(sf::Color(210, 190, 80, 220));
        rollingText.setPosition(static_cast<float>(tsize.x) * 0.35f, TURN_NOTICE_Y);
        window.draw(rollingText);
    }

    // First-roll bust bonus message.
    if (!animating && game.getBustBonusUsedThisTurn() && game.getRollCountThisTurn() == 1 && player->getScore().getRoundScore() >= 50) {
        const sf::Vector2u tsize = window.getSize();
        sf::Text bonusText("First roll bust bonus: +50 points. Roll again.", uiFont, NOTICE_FONT_SIZE);
        bonusText.setFillColor(sf::Color(230, 210, 80, 240));
        bonusText.setPosition(static_cast<float>(tsize.x) * 0.28f, FIRST_BUST_NOTICE_Y);
        window.draw(bonusText);
    }

    // Explain a pending or active steal continuation.
    if (!animating && game.hasStealOfferForCurrentPlayer()) {
        std::string stealNotice;
        if (game.canCurrentPlayerSteal()) {
            stealNotice = "Steal available: carry " + std::to_string(game.getStealOfferScore()) +
                          " points on " + std::to_string(game.getStealOfferDiceCount()) + " dice, or roll fresh.";
        } else {
            stealNotice = "Steal locked until your banked score reaches the Opening Score of " +
                          std::to_string(game.getRuleConfig().getOpeningScoreLimit()) + ". Roll fresh.";
        }
        sf::Text stealText(stealNotice, uiFont, SCORE_FONT_SIZE);
        stealText.setFillColor(sf::Color(230, 210, 80, 240));
        stealText.setPosition(BASE_GAP, STEAL_NOTICE_Y);
        window.draw(stealText);
    } else if (!animating && game.isStealContinuationActive()) {
        sf::Text stealText("Steal continuation active: the carried round score is at risk.", uiFont, SCORE_FONT_SIZE);
        stealText.setFillColor(sf::Color(230, 210, 80, 240));
        stealText.setPosition(BASE_GAP, STEAL_NOTICE_Y);
        window.draw(stealText);
    }

    // Prompt user when awaiting a score selection and no primary actions are visible.
    if (!player->isComputer() && awaitingSelection && primaryButtons.empty() && !game.getBustPending() && !animating) {
        const sf::Vector2u tsize = window.getSize();
        sf::Text prompt("Select a score option", uiFont, NOTICE_FONT_SIZE);
        prompt.setFillColor(sf::Color(210, 190, 80, 220));
        prompt.setPosition(static_cast<float>(tsize.x) * 0.35f, TURN_BTN_ROLL_Y);
        window.draw(prompt);
    }

    // Cache scoreboard values so bust-related changes don't appear until animations finish.
    static std::vector<uint32_t> cachedRoundScores;
    static std::vector<uint32_t> cachedPermanentScores;
    const std::vector<Player>& players = game.getPlayers();
    if (cachedRoundScores.size() != players.size()) {
        cachedRoundScores.assign(players.size(), 0);
        cachedPermanentScores.assign(players.size(), 0);
    }
    if (!animating) {
        for (std::size_t idx = 0; idx < players.size(); ++idx) {
            cachedRoundScores[idx] = players[idx].getScore().getRoundScore();
            cachedPermanentScores[idx] = players[idx].getScore().getPermanentScore();
        }
    }

    // Draw scorecard for all players in top-right using cached values.
    float scorecardY = SCORECARD_TOP_Y;
    for (std::size_t idx = 0; idx < players.size(); ++idx) {
        const Player& p = players[idx];
        const bool isCurrent = (idx == game.getCurrentIndex());
        const uint32_t displayRound = cachedRoundScores[idx];
        const uint32_t displayPermanent = cachedPermanentScores[idx];
        const uint32_t total = displayPermanent + (isCurrent ? displayRound : 0);

        // Player name line
        sf::Text line;
        line.setFont(uiFont);
        line.setCharacterSize(SCORECARD_FONT_SIZE);
        line.setFillColor(isCurrent ? COLOR_TEXT_PRIMARY : COLOR_TEXT_MUTED);
        line.setStyle(isCurrent ? sf::Text::Bold : sf::Text::Regular);

        // Name
        line.setString(p.getName());
        line.setPosition(SCORECARD_RIGHT_X, scorecardY);
        window.draw(line);

        // Round score
        sf::Text totalText(std::to_string(total), uiFont, SCORECARD_FONT_SIZE);
        totalText.setFillColor(isCurrent ? COLOR_TEXT_PRIMARY : COLOR_TEXT_MUTED);
        totalText.setStyle(isCurrent ? sf::Text::Bold : sf::Text::Regular);
        totalText.setPosition(SCORECARD_RIGHT_X + SCORECARD_SCORE_OFFSET, scorecardY);
        window.draw(totalText);

        // Advance to next line
        scorecardY += SCORECARD_LINE_STEP;
    }

    // Render a collection of buttons.
    auto drawButtons = [this](const std::vector<Button>& buttons)->void {
        for (const Button& b : buttons) {
            // Skip invisible buttons.
            if (!b.visible) continue;

            // Draw button rectangle.
            sf::RectangleShape rect({b.w, b.h});
            rect.setPosition(b.x, b.y);
            rect.setFillColor(b.color);
            rect.setOutlineThickness(BUTTON_OUTLINE_THICKNESS);
            rect.setOutlineColor(COLOR_BUTTON_OUT);
            window.draw(rect);

            // Draw button label.
            sf::Text label(b.text, uiFont, BUTTON_FONT_SIZE);
            label.setFillColor(b.textColor);
            label.setPosition(b.x + BUTTON_LABEL_OFFSET_X, b.y + BUTTON_LABEL_OFFSET_Y);
            window.draw(label);
        }
    };

    // Draw controls for a human, or a paced status line while a computer acts.
    if (!animating) {
        if (player->isComputer()) {
            const std::string status = std::string(zilch::computer::difficultyName(player->getComputerDifficulty())) +
                                       " computer is playing...";
            sf::Text computerStatus(status, uiFont, NOTICE_FONT_SIZE);
            computerStatus.setFillColor(sf::Color(230, 210, 80, 240));
            computerStatus.setPosition(UI_CENTER_X - 150.f, TURN_NOTICE_Y);
            window.draw(computerStatus);
        } else {
            drawButtons(primaryButtons);
            drawButtons(optionButtons);
        }
    }

    // Render dice on top of the UI layer.
    diceRenderer.draw(window, &uiFont);

    // End turn rendering.
    window.popGLStates();
}

/**
 * @brief Renders the game over screen displaying the winner and a button to close the game.
 *
 * @param game Reference to the GameManager instance.
 * @param events Vector of SFML events to handle user input.
 */
void GameUI::renderGameOver(const GameManager& game, const std::vector<sf::Event>& events)
{
    Button closeBtn = makeButton(
        GAMEOVER_CLOSE_X,
        GAMEOVER_CLOSE_Y,
        BUTTON_WIDTH,
        BUTTON_HEIGHT,
        "Close",
        [this]()->void { window.close(); },
        COLOR_BUTTON_CLOSE,
        COLOR_TEXT_PRIMARY
    );

    // Handle close button clicks.
    for (const sf::Event& e : events)
        if (e.type == sf::Event::MouseButtonReleased && e.mouseButton.button == sf::Mouse::Left)
            handleButtonClick(static_cast<float>(e.mouseButton.x), static_cast<float>(e.mouseButton.y), {closeBtn});

    // Begin game over screen rendering.
    window.pushGLStates();
    window.clear(COLOR_BG_GAMEOVER);

    // Determine winner(s).
    const RuleConfig& cfg = game.getRuleConfig();
    const std::vector<Player>& players = game.getPlayers();
    std::string titleText = "Winner: Unknown";
    if (!players.empty()) {
        uint32_t maxScore = 0;
        for (const Player& p : players) maxScore = std::max(maxScore, p.getScore().getPermanentScore());
        std::vector<std::string> winners;
        for (const Player& p : players)
            if (p.getScore().getPermanentScore() == maxScore)
                winners.push_back(p.getName());
        if (winners.size() > 1 && cfg.tiesAllowed()) {
            std::string names;
            for (std::size_t i = 0; i < winners.size(); ++i) {
                if (i) names += ", ";
                names += winners[i];
            }
            titleText = "Winners (tie): " + names;
        } else if (winners.size() > 1 && game.isFinalRoundActive() && game.getFinalRoundLeaderIndex() < players.size()) {
            titleText = "Winner: " + players[game.getFinalRoundLeaderIndex()].getName();
        } else {
            titleText = "Winner: " + winners.front();
        }
    }

    // Title text.
    sf::Text title(titleText, uiFont, GAMEOVER_TITLE_SIZE);
    title.setFillColor(COLOR_TEXT_PRIMARY);
    title.setPosition(GAMEOVER_TITLE_X, GAMEOVER_TITLE_Y);
    window.draw(title);

    // Winner score line.
    const uint32_t winningScore = (!players.empty()) ? std::ranges::max_element(
        players,
        [](const Player& a, const Player& b)->bool {
            return a.getScore().getPermanentScore() < b.getScore().getPermanentScore();
        })->getScore().getPermanentScore() : 0;

    // Score text.
    sf::Text scoreLine("Score: " + std::to_string(winningScore), uiFont, LABEL_FONT_SIZE);
    scoreLine.setFillColor(COLOR_TEXT_MUTED);
    scoreLine.setStyle(sf::Text::Bold);
    scoreLine.setPosition(GAMEOVER_TITLE_X, GAMEOVER_TITLE_Y + GAMEOVER_TITLE_SIZE + 8.f);
    window.draw(scoreLine);

    // Close button.
    sf::RectangleShape buttonShape({closeBtn.w, closeBtn.h});
    buttonShape.setPosition(closeBtn.x, closeBtn.y + 24.f);
    buttonShape.setFillColor(closeBtn.color);
    window.draw(buttonShape);

    // Close button label.
    sf::Text btnLabel(closeBtn.text, uiFont, LABEL_FONT_SIZE);
    btnLabel.setFillColor(closeBtn.textColor);
    btnLabel.setPosition(closeBtn.x + CLOSE_BUTTON_LABEL_OFFSET, closeBtn.y + CLOSE_BUTTON_LABEL_OFFSET + 24.f);
    window.draw(btnLabel);

    // Sorted leaderboard in top-right.
    std::vector<std::pair<std::string, uint32_t>> leaderboard;
    leaderboard.reserve(players.size());
    for (const Player& p : players)
        leaderboard.emplace_back(p.getName(), p.getScore().getPermanentScore());
    std::ranges::sort(
        leaderboard,
        [](const auto& a, const auto& b)->bool {
        return a.second > b.second;
    });

    // Draw leaderboard entries.
    float scorecardY = SCORECARD_TOP_Y;
    for (const auto& [name, total] : leaderboard) {
        // Player name line
        sf::Text line(name, uiFont, SCORECARD_FONT_SIZE);
        line.setFillColor(COLOR_TEXT_PRIMARY);
        line.setStyle(sf::Text::Bold);
        line.setPosition(SCORECARD_RIGHT_X, scorecardY);
        window.draw(line);

        // Total score
        sf::Text totalText(std::to_string(total), uiFont, SCORECARD_FONT_SIZE);
        totalText.setFillColor(COLOR_TEXT_PRIMARY);
        totalText.setStyle(sf::Text::Bold);
        totalText.setPosition(SCORECARD_RIGHT_X + SCORECARD_SCORE_OFFSET, scorecardY);
        window.draw(totalText);

        // Advance to next line
        scorecardY += SCORECARD_LINE_STEP;
    }

    // End game over screen rendering.
    window.popGLStates();
}

/**
 * @brief Creates a button with specified properties.
 *
 * @param x The x-coordinate of the button.
 * @param y The y-coordinate of the button.
 * @param width The width of the button.
 * @param height The height of the button.
 * @param text The text label of the button.
 * @param onClick The function to call when the button is clicked.
 * @param color The background color of the button.
 * @param textColor The text color of the button label.
 *
 * @return The created Button instance.
 */
GameUI::Button GameUI::makeButton(const float x, const float y, const float width, const float height, const std::string& text, const std::function<void()>& onClick, const sf::Color color, const sf::Color textColor)
{
    Button b{x, y, width, height, text, onClick, true, color, textColor};
    return b;
}

/**
 * @brief Handles button click events by checking if the mouse coordinates are within any button's bounds.
 *
 * @param mx The x-coordinate of the mouse click.
 * @param my The y-coordinate of the mouse click.
 * @param buttons The vector of buttons to check against.
 */
void GameUI::handleButtonClick(const float mx, const float my, const std::vector<Button>& buttons)
{
    // Check each button for hit-testing and invoke its action.
    for (const Button& b : buttons) {
        if (!b.visible) continue;
        if (mx >= b.x && mx <= b.x + b.w && my >= b.y && my <= b.y + b.h && b.onClick)
            b.onClick();
    }
}
