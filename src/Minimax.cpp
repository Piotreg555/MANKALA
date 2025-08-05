#include "Minimax.hpp"

#include <ctime>

#include "GameLogic.hpp"



int evaluateBoard(const GameState& state, const bool evaluatingPlayerIsPlayer1) {
    double score = std::numeric_limits<int>::min();
    if (state.config.rules == RuleVariant::KALAH) {
        // === Wagi heurystyk ===
        constexpr double W1 = 0.225;
        constexpr double W2 = 0.122;
        constexpr double W3 = 0.654;
        constexpr double W4 = 1.0;
        constexpr double W5 = 0.484;
        constexpr double W6 = 0.694;
        constexpr double W7 = 0.918;
        constexpr double W8 = 0.667;
        constexpr double W9 = 0.194;
        constexpr double W10 = 0.297;

        const int pitsPerPlayer = state.config.numPitsPerPlayer;

        const bool isPlayer1 = evaluatingPlayerIsPlayer1;
        const int playerOffset = isPlayer1 ? 0 : pitsPerPlayer + 1;
        const int opponentOffset = isPlayer1 ? pitsPerPlayer + 1 : 0;

        const int H1 = state.pits[playerOffset];  // Kamienie w pierwszym dołku gracza
        int H2 = 0; // Suma kamieni w dołkach gracza
        int H3 = 0; // Liczba niepustych dołków
        for (int i = 0; i < pitsPerPlayer; ++i) {
            const int stones = state.pits[playerOffset + i];
            H2 += stones;
            if (stones > 0) ++H3;
        }

        const int H4 = state.pits[playerOffset + pitsPerPlayer]; // Magazyn gracza

        // H5: Czy ruch z najbardziej prawym dołka jest możliwy
        const int H5 = (state.pits[playerOffset + pitsPerPlayer - 1] > 0) ? 1 : 0;

        // H6: Ujemna wartość magazynu przeciwnika
        const int H6 = -state.pits[opponentOffset + pitsPerPlayer];

        // H7: Czy ruch nie kończy gry
        int H7 = 0;
        for (int i = 0; i < pitsPerPlayer; ++i) {
            if (state.pits[playerOffset + i] > 0) {
                GameState sim = state;
                sim = makeMove(sim, playerOffset + i);
                // Jeśli gra się nie zakończyła (czyli są jeszcze ruchy)
                bool hasMove = false;
                for (int j = 0; j < pitsPerPlayer; ++j) {
                    if (sim.isPlayerOneTurn == isPlayer1 && sim.pits[playerOffset + j] > 0) {
                        hasMove = true;
                        break;
                    }
                }
                H7 = hasMove ? 1 : 0;
                break;
            }
        }

        // H8: różnica między magazynami
        const int playerStore = state.pits[playerOffset + pitsPerPlayer];
        const int opponentStore = state.pits[opponentOffset + pitsPerPlayer];
        const int H8 = playerStore - opponentStore;

        // H9: kara za przeciwnika mającego dużo w magazynie
        double H9 = 0;
        if (opponentStore >= 5) {
            H9 = - (opponentStore * 1.5) - playerStore;
        }

        // H10: bonus, jeśli gracz ma dużo w magazynie
        double H10 = 0;
        if (playerStore >= 5) {
            H10 = (playerStore * 1.5) - opponentStore;
        }

        // Sumowanie heurystyk
        score = H1 * W1 + H2 * W2 + H3 * W3 + H4 * W4 +
                       H5 * W5 + H6 * W6 + H7 * W7 + H8 * W8 +
                       H9 * W9 + H10 * W10;
    }
    if (state.config.rules == RuleVariant::WARI) {

    }
    return static_cast<int>(score);
}

int minimax(const GameState& state, const int depth, const bool maximizingPlayer, bool evaluatingPlayerIsPlayer1) {
    if (depth == 0 || isGameOver(state)) {
        return evaluateBoard(state, evaluatingPlayerIsPlayer1);
    }

    auto movesWithStates = getAvailableMovesWithStates(state);
    if (movesWithStates.empty()) {
        return evaluateBoard(state, evaluatingPlayerIsPlayer1); // Gra zakończona lub brak ruchów
    }

    if (maximizingPlayer) {
        int maxEval = std::numeric_limits<int>::min();
        for (const auto &nextState: movesWithStates | std::views::values) {
            int eval = minimax(nextState, depth - 1, nextState.isPlayerOneTurn == evaluatingPlayerIsPlayer1, evaluatingPlayerIsPlayer1);
            maxEval = std::max(maxEval, eval);
        }
        return maxEval;
    } else {
        int minEval = std::numeric_limits<int>::max();
        for (const auto &nextState: movesWithStates | std::views::values) {
            int eval = minimax(nextState, depth - 1, nextState.isPlayerOneTurn == evaluatingPlayerIsPlayer1, evaluatingPlayerIsPlayer1);
            minEval = std::min(minEval, eval);
        }
        return minEval;
    }
}

std::pair<int, GameState> findBestMove(const GameState& state, std::vector<std::pair<int, GameState> > &movesWithStates, const int depth) {
    std::vector<std::pair<int, GameState>> bestMoves;
    int bestScore = std::numeric_limits<int>::min();

    for (const auto& [move, nextState] : movesWithStates) {
        if (const int score = minimax(nextState, depth - 1, nextState.isPlayerOneTurn == state.isPlayerOneTurn, state.isPlayerOneTurn); score > bestScore) {
            bestScore = score;
            bestMoves.clear();
            bestMoves.emplace_back(move, nextState);
        } else if (score == bestScore) {
            bestMoves.emplace_back(move, nextState);
        }
    }

    if (bestMoves.empty()) {
        return {-1, state}; // brak dostępnych ruchów
    }

    // RNG do losowego wyboru najlepszego ruchu
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, bestMoves.size() - 1);

    return bestMoves[dist(gen)];
}