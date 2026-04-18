// GameUI.h
/**
Author: Jacob Anderson
Class: ECE-6122-A
Last Date Modified: 12/2/2025

Description:
This file contains the declaration of the GameUI class, which manages the user interface
for the game using SFML. It includes layout constants, typography settings, and methods for rendering
various game screens such as the intro, setup, player turns, and game over.
*/

#ifndef ZILCH_GAME_UI_H
#define ZILCH_GAME_UI_H

#include "DiceRenderer.h"

#include <functional>
#include <string>
#include <vector>
#include <SFML/Graphics.hpp>

/** @class GameManager GameManager.h "GameManager" */
class GameManager;
/** @class DiceRenderer DiceRenderer.h "DiceRenderer" */
class Checker;
/** @class Dice Dice.h "Dice.h" */
class Dice;

/**
 * @class GameUI GameUI.h "GameUI"
 *
 * @brief Manages the user interface for the game using SFML.
 */
class GameUI {
public:
    GameUI();
    void run(GameManager& game);

private:
	// Button representation
	struct Button {
		float x, y, w, h;	// position and size
		std::string text;	// label
		std::function<void()> onClick;	// click handler
		bool visible{true};	// visibility flag
		sf::Color color{sf::Color(70, 130, 240)};	// default button color (Blue)
		sf::Color textColor{sf::Color::White};						// default text color
	};

	// Layout constants
    sf::RenderWindow window;
    sf::Font uiFont;
    DiceRenderer renderer;
    int focusedNameIndex{-1};
    std::string nameEditBuffer;
    bool editingScoreLimit{false};
    bool editingBankThreshold{false};
    std::string scoreLimitBuffer;
    std::string bankThresholdBuffer;

	// Typography constants
    static sf::Color defaultButtonColor();
    static sf::Color defaultButtonTextColor();

	// Rendering methods for different game states
    void renderIntro(
        GameManager& game,
        std::vector<std::string>& playerNames,
        uint32_t& scoreLimit,
        bool& waitingForRoll,
        bool& awaitingSelection,
        std::vector<Button>& optionButtons,
        const std::vector<sf::Event>& events
    );
    void renderRules(GameManager& game, const std::vector<sf::Event>& events);
	void renderSetup(
        GameManager& game, std::vector<std::string>& playerNames, uint32_t& scoreLimit,
        const std::vector<sf::Event>& events
    );
	void renderTurn(
        GameManager& game, const DiceRenderer& diceRenderer, const Checker& checker, std::vector<Button>& optionButtons,
        bool& waitingForRoll, bool& awaitingSelection, const std::vector<sf::Event>& events
    );
    void renderGameOver(const GameManager& game, const std::vector<sf::Event>& events);

    // Button helpers (Input only)
    static void handleButtonClick(float mx, float my, const std::vector<Button>& buttons);
    static Button makeButton(
        float x,
        float y,
        float width,
        float height,
        const std::string& text,
        const std::function<void()>& onClick,
        sf::Color color = defaultButtonColor(),
        sf::Color textColor = defaultButtonTextColor()
    );
};

#endif // ZILCH_GAME_UI_H
