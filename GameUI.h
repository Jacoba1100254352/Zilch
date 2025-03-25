// GameUI.h

#ifndef ZILCH_GAME_UI_H
#define ZILCH_GAME_UI_H

#include <cstdint>

class GameManager;

void displayGameInfo();
void clear();
void pauseAndContinue();
void printInstructions(const GameManager &game, uint16_t options);

#endif //ZILCH_GAME_UI_H
