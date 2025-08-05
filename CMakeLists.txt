cmake_minimum_required(VERSION 3.29)
project(MANKALA)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

add_executable(MANKALA main.cpp
        include/GameTypes.hpp
        include/GameLogic.hpp
        include/Minimax.hpp
        src/GameLogic.cpp
        src/Minimax.cpp
        )
