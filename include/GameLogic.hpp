#pragma once
#include "GameTypes.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <random>
#include <ranges>
#include <string>
#include <sstream>

GameState initializeGame(const GameConfig &config);

std::vector<std::pair<int, GameState> > getAvailableMovesWithStates(const GameState &state);
bool isGameOver(const GameState& state);
GameState makeMove(const GameState &state, int pitIndex);

void simulateGame(GameConfig config, int depthPlayer1 = 6, int depthPlayer2 = 6, int numberOfGames = 1, bool printStats = false,
                  bool printHistory = false, bool showBoard = false);

void printBoard(const GameState &state);
