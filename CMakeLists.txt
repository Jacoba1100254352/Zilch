cmake_minimum_required(VERSION 3.20)
project(Zilch)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-lncurses")

add_executable(Zilch
        main.cpp
        zilch.cpp
        zilch.h)
