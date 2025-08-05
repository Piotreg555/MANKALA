#include "GameLogic.hpp"
#include "Minimax.hpp"
#include <chrono>
// --- Inicjalizacja gry ---
GameState initializeGame(const GameConfig &config) {
    GameState state;
    state.config = config;
    int totalPits = config.numPitsPerPlayer * 2 + 2; // dołki obu graczy + 2 magazyny
    state.pits.resize(totalPits, config.stonesPerPit);
    // Zerujemy magazyny
    state.pits[config.numPitsPerPlayer] = 0; // magazyn gracza 1
    state.pits[config.numPitsPerPlayer * 2 + 1] = 0; // magazyn gracza 2
    state.isPlayerOneTurn = true;
    return state;
}

// --- Wyświetlanie planszy ---
void printBoard(const GameState &state) {
    const int n = state.config.numPitsPerPlayer;

    // Indeksy:
    // P1 dołki: 0...n-1
    // P1 magazyn: n
    // P2 dołki: n+1..2n
    // P2 magazyn: 2n+1
    //std::cout << "      D6   D5   D4   D3   D2   D1\n";
    std::cout << "P2  ";
    // Górny rząd: dołki gracza 2 (w kolejności odwrotnej)
    for (int i = 2 * n; i > n; --i) {
        std::cout << (state.pits[i] < 100 ? "[ " : "[") << state.pits[i] << (state.pits[i] < 10 ? " ]" : "]");
    }
    std::cout << "\n";

    // Magazyny po bokach
    std::cout << (state.pits[2 * n + 1] < 100 ? "[ " : "[") << state.pits[2 * n + 1] << (
        state.pits[2 * n + 1] < 10 ? " ]" : "]"); // magazyn P2
    std::cout << std::string(n * 5 - 2, ' '); // odstęp
    std::cout << (state.pits[n] < 100 ? "[ " : "[") << state.pits[n] << (state.pits[n] < 10 ? " ]" : "]") << std::endl;
    // magazyn P1

    // Dolny rząd: dołki gracza 1 (normalna kolejność)
    std::cout << "P1  ";
    for (int i = 0; i < n; ++i) {
        std::cout << (state.pits[i] < 100 ? "[ " : "[") << state.pits[i] << (state.pits[i] < 10 ? " ]" : "]");
    }
    //std::cout << "\n      D1   D2   D3   D4   D5   D6";
    std::cout << "\n";
}

bool isGameOver(const GameState& state) {
    const int pitsPerPlayer = state.config.numPitsPerPlayer;
    const int totalStones = state.config.stonesPerPit * 2 * pitsPerPlayer;

    const int player1StoreIndex = pitsPerPlayer;
    const int player2StoreIndex = 2 * pitsPerPlayer + 1;

    const int player1Stones = state.pits[player1StoreIndex];
    const int player2Stones = state.pits[player2StoreIndex];

    // Warunek 1: Któryś z graczy ma > połowę kamieni
    if (player1Stones > totalStones / 2 || player2Stones > totalStones / 2) {
        return true;
    }

    // Warunek 2: Gracz, który ma teraz ruch, nie ma dostępnych ruchów
    const int offset = state.isPlayerOneTurn ? 0 : pitsPerPlayer + 1;
    bool hasMove = false;
    for (int i = 0; i < pitsPerPlayer; ++i) {
        if (state.pits[offset + i] > 0) {
            hasMove = true;
            break;
        }
    }

    return !hasMove;
}

GameState makeMove(const GameState &state, const int pitIndex) {
    GameState newState = state; // kopia stanu, żeby nie modyfikować oryginału
    int stones = newState.pits[pitIndex];
    newState.pits[pitIndex] = 0;
    int pos = pitIndex;
    const int n = newState.config.numPitsPerPlayer;

    bool captureOccurred = false;
    bool extraMove = false;

    while (stones > 0) {
        pos = (pos + 1) % static_cast<int>(newState.pits.size());

        // Pomijamy magazyn przeciwnika
        if (newState.isPlayerOneTurn && pos == n * 2 + 1) continue;
        if (!newState.isPlayerOneTurn && pos == n) continue;
        // Pomijamy dołek startowy
        if (pos == pitIndex) continue;

        // W wariancie WARI pomijamy własną mankalę
        if (newState.config.rules == RuleVariant::WARI && newState.isPlayerOneTurn && pos == n) continue;
        if (newState.config.rules == RuleVariant::WARI && !newState.isPlayerOneTurn && pos == n * 2 + 1) continue;

        newState.pits[pos]++;
        stones--;
    }

    // --- Zbijanie w wariancie WARI ---
    if (newState.config.rules == RuleVariant::WARI) {
        int start = pos;

        // Sprawdzamy tylko po stronie przeciwnika
        const int opponentStart = newState.isPlayerOneTurn ? n + 1 : 0;
        const int opponentEnd = newState.isPlayerOneTurn ? 2 * n + 1 : n;

        while (start >= opponentStart && start < opponentEnd) {
            if (newState.pits[start] == 2 || newState.pits[start] == 3) {
                // Zbijamy
                const int captured = newState.pits[start];
                newState.pits[start] = 0;

                // Dodajemy do magazynu gracza
                const int store = newState.isPlayerOneTurn ? n : 2 * n + 1;
                newState.pits[store] += captured;

                captureOccurred = true;
                start--; // sprawdzamy kolejny dołek na lewo
            } else {
                break;
            }
        }
    }
    if (newState.config.rules == RuleVariant::KALAH) {
        // Sprawdzamy, czy ostatni kamień wylądował po stronie gracza
        const int playerStart = newState.isPlayerOneTurn ? 0 : n + 1;
        const int playerEnd = newState.isPlayerOneTurn ? n : 2 * n + 1;

        if (pos >= playerStart && pos < playerEnd && newState.pits[pos] == 1) {
            // Indeks dołka przeciwnika "naprzeciwko"
            int opposite = 2 * n - pos;

            if (newState.pits[opposite] > 0) {
                int captured = newState.pits[opposite] + 1; // przeciwnik + ostatni własny
                newState.pits[opposite] = 0;
                newState.pits[pos] = 0;

                const int store = newState.isPlayerOneTurn ? n : 2 * n + 1;
                newState.pits[store] += captured;

                captureOccurred = true;
            }
        }
        if (const int playerStore = newState.isPlayerOneTurn ? n : 2 * n + 1; pos == playerStore) {
            extraMove = true;
        }
    }

    if (captureOccurred) {
        state.movesWithoutCapture = 0;
    } else {
        state.movesWithoutCapture++;
    }

    if (!extraMove) newState.isPlayerOneTurn = !newState.isPlayerOneTurn;
    return newState;
}

std::vector<std::pair<int, GameState> > getAvailableMovesWithStates(const GameState &state) {
    std::vector<std::pair<int, GameState> > legalMoves;

    int n = state.config.numPitsPerPlayer;
    int start = state.isPlayerOneTurn ? 0 : n + 1;
    int end = state.isPlayerOneTurn ? n : 2 * n + 1;

    for (int i = start; i < end; ++i) {
        if (state.pits[i] > 0) {
            GameState nextState = makeMove(state, i);

            if (state.config.rules == RuleVariant::WARI) {
                // --- Sprawdzenie: czy przeciwnik będzie miał kamienie ---
                int opponentStart = state.isPlayerOneTurn ? n + 1 : 0;
                int opponentEnd = state.isPlayerOneTurn ? 2 * n + 1 : n;
                bool opponentHasStones = false;

                for (int j = opponentStart; j < opponentEnd; ++j) {
                    if (nextState.pits[j] > 0) {
                        opponentHasStones = true;
                        break;
                    }
                }

                if (opponentHasStones) {
                    legalMoves.emplace_back(i, nextState);
                }
            }
            if (state.config.rules == RuleVariant::KALAH) {
                legalMoves.emplace_back(i, nextState);
            }
        }
    }
    return legalMoves;
}

std::pair<int, GameState> choosePit(std::vector<std::pair<int, GameState> > &movesWithStates, const GameState &state,
                                    int depth) {
    const Player currentPlayer = (state.isPlayerOneTurn ? state.config.Player1 : state.config.Player2);
    if (currentPlayer == Player::PLAYER) {
        printBoard(state);
        std::cout << "Player to move: " << (state.isPlayerOneTurn ? "1" : "2") << "\n";

        std::vector<int> moves;
        for (const auto &key: movesWithStates | std::views::keys) {
            moves.push_back(key);
        }

        int pitIndex;
        while (true) {
            std::cout << "Choose pit (1-" << state.config.numPitsPerPlayer << ") from available: ";
            for (auto m: moves) {
                int userIndex;
                if (state.isPlayerOneTurn)
                    userIndex = m + 1; // gracz 1: indeks +1
                else
                    userIndex = m - (state.config.numPitsPerPlayer + 1) + 1; // gracz 2: indeks przesunięty

                std::cout << userIndex << " ";
            }
            std::cout << "\n> ";

            int choice;
            std::cin >> choice;
            if (choice < 1 || choice > state.config.numPitsPerPlayer) {
                std::cout << "Invalid choice. Try again.\n";
                continue;
            }


            if (state.isPlayerOneTurn)
                pitIndex = choice - 1;
            else
                pitIndex = choice + state.config.numPitsPerPlayer;

            if (std::ranges::find(moves, pitIndex) != moves.end()) {
                break;
            }
            std::cout << "Invalid choice. Try again.\n";
        }
        return *std::ranges::find_if(movesWithStates
                                     ,
                                     [pitIndex](const std::pair<int, GameState> &entry) {
                                         return entry.first == pitIndex;
                                     });
    }

    if (currentPlayer == Player::RANDOM) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<std::size_t> dist(0, movesWithStates.size() - 1);

        const std::size_t randomIndex = dist(gen);
        return movesWithStates[randomIndex];
    }
    if (currentPlayer == Player::COMPUTER) {
        return findBestMove(state, movesWithStates, depth);
    }
    return movesWithStates[state.config.numPitsPerPlayer]; //do implementacji
}

void simulateGame(GameConfig config, int depthPlayer1, int depthPlayer2, int numberOfGames, bool printStats,
                  bool printHistory,
                  bool showBoard) {
    std::ostringstream filename;
    filename << config.rulesName() << "_" << config.numPitsPerPlayer << "_" << config.stonesPerPit << "_" << config.
            Player1Name();
    if (config.Player1Name() == "C") filename << depthPlayer1;
    filename << "v" << config.Player2Name();
    if (config.Player2Name() == "C") filename << depthPlayer2;
    filename << "_1e" << log10(numberOfGames) << "g";
    std::cout << filename.str() << ": 0% ";
    filename << ".txt";

    std::ofstream file(filename.str()); // tworzy i otwiera plik do zapisu
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filename.str() << std::endl;
        return;
    }

    file << config.rulesName() << ": " << config.numPitsPerPlayer << " pits_per_player, " << config.stonesPerPit <<
            " stones_per_pit, P1: " << config.Player1Name();
    if (config.Player1Name() == "C") file << "(depth: " << depthPlayer1 << ")";
    file << ", P2: " << config.Player2Name();
    if (config.Player2Name() == "C") file << "(depth: " << depthPlayer2 << ")";
    file << ", " << numberOfGames << " games\n";
    if (printHistory) file << std::endl << "pit_sequence;";
    if (printHistory || printStats) file << "p1_score;p2_score;number_of_moves;\n";

    int p1Wins = 0;
    int p2Wins = 0;
    int draws = 0;
    int loops = 0;

    int totalNumberOfMoves = 0;
    int longestGame = 0;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 1; i <= numberOfGames; i++) {
        float progress = static_cast<float>(i) / static_cast<float>(numberOfGames) * 100;
        if (std::fmod(progress, 10) < 1e-5) std::cout << progress << "% ";
        GameState state = initializeGame(config);
        int numberOfMoves = 0;
        bool loop = false;
        int p1Score;
        int p2Score;
        bool earlyEnd = false;
        while (true) {
            std::vector<std::pair<int, GameState> > movesWithStates = getAvailableMovesWithStates(state);
            if (movesWithStates.empty()) break;

            int n = state.config.numPitsPerPlayer;
            if (state.movesWithoutCapture == 1000) {
                // Sumujemy wszystkie kamienie na planszy oprócz magazynów

                int totalStones = 0;
                for (int j = 0; j < static_cast<int>(state.pits.size()); ++j) {
                    // Pomijamy magazyny
                    if (j == n || j == 2 * n + 1) continue;
                    totalStones += state.pits[j];
                    state.pits[j] = 0;
                }

                // Jeśli liczba kamieni jest nieparzysta, wyrzucamy 1 kamień
                if (totalStones % 2 != 0) {
                    totalStones -= 1;
                }

                // Rozdzielamy po równo między magazyny
                int half = totalStones / 2;
                state.pits[n] += half; // magazyn gracza 1
                state.pits[2 * n + 1] += half; // magazyn gracza 2

                loop = true;
                loops++;
                break;
            }

            const auto [pitIndex, newState] = choosePit(movesWithStates, state,
                                                        state.isPlayerOneTurn ? depthPlayer1 : depthPlayer2);
            if (showBoard) {
                std::cout << std::endl << pitIndex << std::endl;
                printBoard(newState);
            }
            if (printHistory) {
                file << pitIndex << ",";
            }
            state = newState;
            numberOfMoves++;

            p1Score = state.pits[n];
            p2Score = state.pits[2 * n + 1];
            if (p1Score > n * state.config.stonesPerPit ||
                p2Score > n * state.config.stonesPerPit) {
                earlyEnd = true;
                break;
            }
        }
        if (printHistory) file << ";";

        if (state.config.rules == RuleVariant::KALAH && !earlyEnd) {
            p1Score = std::accumulate(state.pits.begin(),
                                      state.pits.begin() + static_cast<std::vector<int>::difference_type>(
                                          state.pits.size() / 2),
                                      0);
            p2Score = std::accumulate(
                state.pits.begin() + static_cast<std::vector<int>::difference_type>(state.pits.size() / 2),
                state.pits.end(),
                0);
        }
        if (p1Score > p2Score) p1Wins++;
        if (p2Score > p1Score) p2Wins++;
        if (p1Score == p2Score) draws++;
        if (printHistory || printStats) {
            file << p1Score << ";" << p2Score << ";" << numberOfMoves << ";";

            if (loop) {
                file << "LOOP";
            }
            file << std::endl;
        }
        if (!loop) {
            if (numberOfMoves > longestGame) longestGame = numberOfMoves;
            totalNumberOfMoves += numberOfMoves;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = end - start;

    file << "\nP1's wins: " << p1Wins << std::endl
            << "P2's wins: " << p2Wins << std::endl
            << "Draws: " << draws << std::endl
            << "\nLoops: " << loops << std::endl
            << "\nExcluding games with loops:\nAverage number of moves: " << totalNumberOfMoves / numberOfGames <<
            std::endl
            << "The longest game: " << longestGame << " moves" << std::endl;
    file << "\nExecution time: " << elapsed.count() << " s";
    file.close();
    std::cout << " - Finished. Check file for results." << std::endl;
}
