cmake_minimum_required(VERSION 3.10)
project(Zilch)
include(CTest)

if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(APPLE AND EXISTS "/opt/homebrew/opt/sfml@2")
    list(PREPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/sfml@2")
elseif(APPLE AND EXISTS "/opt/homebrew/opt/sfml")
    list(PREPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/sfml")
endif()

if(APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND EXISTS "/opt/homebrew/opt/libomp")
    set(OpenMP_ROOT "/opt/homebrew/opt/libomp")
    list(PREPEND CMAKE_PREFIX_PATH "${OpenMP_ROOT}")
endif()

find_package(SFML 2.6 REQUIRED COMPONENTS graphics window system)
find_package(OpenMP)

add_executable(Zilch
        main.cpp
        Dice.cpp
        Checker.cpp
        GameManager.cpp
        GameUI.cpp
        DiceRenderer.cpp
        ResourcePath.cpp
        GameStateMachine.cpp
        common/objloader.cpp)

target_include_directories(Zilch PRIVATE
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/common
        ${PROJECT_SOURCE_DIR}/external/glm-0.9.7.1)

target_link_libraries(Zilch PRIVATE
        sfml-graphics
        sfml-window
        sfml-system)

if(OpenMP_CXX_FOUND)
    target_link_libraries(Zilch PRIVATE OpenMP::OpenMP_CXX)
endif()

foreach(asset_dir fonts graphics sound)
    add_custom_command(TARGET Zilch POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/${asset_dir}"
            "$<TARGET_FILE_DIR:Zilch>/${asset_dir}")
endforeach()

if(MSVC)
    target_compile_options(Zilch PRIVATE /W4 /permissive-)
else()
    target_compile_options(Zilch PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(BUILD_TESTING)
    add_executable(ZilchRulesTests
            tests/test_game_rules.cpp
            Dice.cpp
            Checker.cpp
            GameManager.cpp
            GameStateMachine.cpp)

    target_include_directories(ZilchRulesTests PRIVATE ${PROJECT_SOURCE_DIR})

    if(MSVC)
        target_compile_options(ZilchRulesTests PRIVATE /W4 /permissive-)
    else()
        target_compile_options(ZilchRulesTests PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME ZilchRulesTests COMMAND ZilchRulesTests)
endif()
