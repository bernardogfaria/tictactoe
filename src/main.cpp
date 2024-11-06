#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <array>

// Classe TicTacToe
class TicTacToe {
private:
    std::array<std::array<char, 3>, 3> board; // Tabuleiro do jogo
    std::mutex board_mutex; // Mutex para controle de acesso ao tabuleiro
    std::condition_variable turn_cv; // Variável de condição para alternância de turnos
    char current_player; // Jogador atual ('X' ou 'O')
    bool game_over; // Estado do jogo
    char winner; // Vencedor do jogo

public:
    TicTacToe() : current_player('X'), game_over(false), winner(' ') {
        // Inicializar o tabuleiro com espaços vazios
        for (auto &row : board) row.fill(' ');
    }

    void display_board() {
        std::lock_guard<std::mutex> lock(board_mutex);
        for (const auto &row : board) {
            for (const char &cell : row) {
                std::cout << cell << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    bool make_move(char player, int row, int col) {
        std::unique_lock<std::mutex> lock(board_mutex);
        turn_cv.wait(lock, [&] { return current_player == player && !game_over; });

        if (board[row][col] == ' ') {
            board[row][col] = player;
            display_board();
            if (check_win(player)) {
                winner = player;
                game_over = true;
            } else if (check_draw()) {
                winner = 'D';
                game_over = true;
            }
            current_player = (current_player == 'X') ? 'O' : 'X';
            lock.unlock();
            turn_cv.notify_all();
            return true;
        }
        return false;
    }

    bool check_win(char player) {
        for (int i = 0; i < 3; ++i) {
            if ((board[i][0] == player && board[i][1] == player && board[i][2] == player) ||
                (board[0][i] == player && board[1][i] == player && board[2][i] == player)) {
                return true;
            }
        }
        return (board[0][0] == player && board[1][1] == player && board[2][2] == player) ||
               (board[0][2] == player && board[1][1] == player && board[2][0] == player);
    }

    bool check_draw() {
        for (const auto &row : board) {
            for (const char &cell : row) {
                if (cell == ' ') {
                    return false;
                }
            }
        }
        return true;
    }

    bool is_game_over() {
        return game_over;
    }

    char get_winner() {
        return winner;
    }
};

// Classe Player
class Player {
private:
    TicTacToe &game; // Referência para o jogo
    char symbol; // Símbolo do jogador ('X' ou 'O')
    std::string strategy; // Estratégia do jogador

public:
    Player(TicTacToe &g, char s, std::string strat) : game(g), symbol(s), strategy(strat) {}

    void play() {
        if (strategy == "sequential") {
            play_sequential();
        } else if (strategy == "random") {
            play_random();
        }
    }

private:
    void play_sequential() {
        for (int row = 0; row < 3 && !game.is_game_over(); ++row) {
            for (int col = 0; col < 3 && !game.is_game_over(); ++col) {
                game.make_move(symbol, row, col);
            }
        }
    }

    void play_random() {
        while (!game.is_game_over()) {
            int row = rand() % 3;
            int col = rand() % 3;
            game.make_move(symbol, row, col);
        }
    }
};

// Função principal
int main() {
    TicTacToe game;
    Player player1(game, 'X', "sequential");
    Player player2(game, 'O', "random");

    std::thread t1(&Player::play, &player1);
    std::thread t2(&Player::play, &player2);

    t1.join();
    t2.join();

    if (game.get_winner() == 'D') {
        std::cout << "It's a draw!" << std::endl;
    } else {
        std::cout << "Player " << game.get_winner() << " wins!" << std::endl;
    }

    return 0;
}
