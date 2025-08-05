#include "GameLogic.hpp"
#include "Minimax.hpp"

int main() {
    simulateGame(GameConfig{6, 4, RuleVariant::KALAH,
        Player::COMPUTER, Player::RANDOM},
        6, 6, 1e3, false);
    return 0;
}