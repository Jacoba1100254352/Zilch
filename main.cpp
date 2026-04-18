// main.cpp
/**
Author: Jacob Anderson
Class: ECE-6122-A
Last Date Modified: 12/2/2025

Description:
    This file contains the main entry point for the game application.
    It initializes the game manager and user interface, then starts the main UI loop.
*/

#include "GameManager.h"
#include "GameUI.h"

int main()
{
    // Create game manager and UI instances
    GameManager game;
    GameUI ui;

    // Start the main UI loop
    ui.run(game);

    // Exit the program
    return 0;
}
