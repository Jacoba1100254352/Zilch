#ifndef ZILCH_GAME_UI_H
#define ZILCH_GAME_UI_H

#include <map>
#include <string>
#include <cstdint>
#include <iostream>
#include "PlatformDefinitions.h"

class GameManager;

void displayGameInfo();
void clear();
void pauseAndContinue();
void printInstructions(const GameManager &game, uint16_t options);

#endif //ZILCH_GAME_UI_H
