// GameUI.cpp

#include "GameUI.h"
#include "GameManager.h"
#include "PlatformDefinitions.h"

void displayGameInfo() {
    clear();
    std::cout << "Welcome to Zilch!" << std::endl
              << std::endl
              << "Here are the basic rules:" << std::endl
              << "1. You must score an initial 1000 points to start logging your points." << std::endl
              << "2. Sets (three sets of two) and straits (1, 2, 3, 4, 5, 6) give 1000 points." << std::endl
              << "3. A group of 3 identical dice give you 100 points times the value of that die. For example:" << std::endl
              << "\t- A roll of 3 3 3 will give you 300 points." << std::endl
              << "\t- An exception is made for a roll of 1 1 1, which will give you 1000 points." << std::endl
              << "4. Each additional number added to this group of three doubles the points received from it. For example:" << std::endl
              << "\t- A roll of 3 3 3 3 or a roll of 3 3 3 and a later roll of 3 in the same turn would give you 600 points." << std::endl
              << "\t- If in a group of 3 1's, 2 more 1's are rolled, the score would be 1000*2*2 = 4000." << std::endl
              << "5. Finally, only single 1's or 5's are worth points with a single 1 being 100 points and a 5 being 50." << std::endl
              << std::endl;
}

void clear() {
    std::cout << "\033[2J\033[1;1H";
}

void pauseAndContinue() {
    std::cout << "\nPress enter to continue... " << std::flush;
    if (strcmp(PLATFORM_NAME, "windows") > 0)
        system("pause"); // For Windows users
    else if (strcmp(PLATFORM_NAME, "macos") > 0)
        system("read"); // For  macOS users

    clear();
}

