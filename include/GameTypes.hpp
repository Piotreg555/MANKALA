#pragma once
#include <vector>
#include <memory>


enum class RuleVariant {
    WARI,
    KALAH
};

enum class Player {
    RANDOM,
    PLAYER,
    COMPUTER
};

struct GameConfig {
    int numPitsPerPlayer;
    int stonesPerPit;
    RuleVariant rules;
    Player Player1;
    Player Player2;

    [[nodiscard]] std::string Player1Name() const {
        switch (Player1) {
            case Player::RANDOM: return "R";
            case Player::PLAYER: return "P";
            case Player::COMPUTER: return "C";
        }
        return "";
    }

    [[nodiscard]] std::string Player2Name() const {
        switch (Player2) {
            case Player::RANDOM: return "R";
            case Player::PLAYER: return "P";
            case Player::COMPUTER: return "C";
        }
        return "";
    }

    [[nodiscard]] std::string rulesName() const {
        switch (rules) {
            case RuleVariant::WARI: return "Wari";
            case RuleVariant::KALAH: return "Kalah";
        }
        return "";
    }
};

struct GameState {
    std::vector<int> pits; // [P1 dołki...][P1 magazyn][P2 dołki...][P2 magazyn]
    bool isPlayerOneTurn;
    GameConfig config;
    mutable int movesWithoutCapture = 0;
};

struct MinimaxNode {
    GameState state;
    int move = -1;
    int score = 0;
    std::vector<std::shared_ptr<MinimaxNode> > children;
};
