cmake_minimum_required(VERSION 3.30.5)
project(Zilch)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(Zilch
        main.cpp
        Dice.h
        Dice.cpp
        Checker.h
        Checker.cpp
        GameManager.h
        GameManager.cpp
        Player.h
        Score.h
        GameUI.h
        GameUI.cpp
        PlatformDefinitions.h
        ZilchStates.h)
