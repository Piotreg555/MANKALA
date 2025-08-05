#pragma once
#include "GameTypes.hpp"

// === Minimax ===
int evaluateBoard(const GameState& state, bool evaluatingPlayerIsPlayer1);
std::shared_ptr<MinimaxNode> minimaxTree(GameState state, int depth, bool maximizingPlayer);
int minimax(const GameState& state, std::vector<std::pair<int, GameState> > &movesWithStates, int depth, bool maximizingPlayer, bool evaluatingPlayerIsPlayer1);
std::pair<int, GameState> findBestMove(const GameState& state, std::vector<std::pair<int, GameState> > &movesWithStates, int depth);



