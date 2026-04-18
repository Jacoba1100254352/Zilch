// Main.cpp

#include <exception>
#include <iostream>
#include "GameManager.h"
#include "InputClosed.h"

int main() {
    try {
        // Create an instance of the ZilchGame class
        GameManager game;
        game.playGame();
        return 0;
    } catch (const InputClosed&) {
        std::cout << "\nInput closed. Exiting Zilch." << std::endl;
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "\nFatal error: " << error.what() << std::endl;
        return 1;
    }
}
