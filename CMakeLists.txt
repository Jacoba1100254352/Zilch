cmake_minimum_required(VERSION 3.17)
project(Zilch)

set(CMAKE_CXX_STANDARD 23)

add_executable(Zilch
        main.cpp
        oldZilch.h
        Dice.h
        Dice.cpp
        Checker.h
        Checker.cpp
        Zilch.h
        Zilch.cpp Player.h Score.h)
