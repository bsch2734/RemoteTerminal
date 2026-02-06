#include <iostream>
#include <string>
#include <sstream>
#include "TicTacToeEngine.h"

using namespace TicTacToe;

char cellToChar(Player p, int index) {
    switch (p) {
        case Player::one: return 'X';
        case Player::two: return 'O';
        default: return '0' + index;
    }
}

void printBoard(const TicTacToeEngine& engine) {
    const auto& board = engine.getBoard();
    std::cout << "\n";
    std::cout << " " << cellToChar(board[0], 0) << " | "
                     << cellToChar(board[1], 1) << " | "
                     << cellToChar(board[2], 2) << " \n";
    std::cout << "---+---+---\n";
    std::cout << " " << cellToChar(board[3], 3) << " | "
                     << cellToChar(board[4], 4) << " | "
                     << cellToChar(board[5], 5) << " \n";
    std::cout << "---+---+---\n";
    std::cout << " " << cellToChar(board[6], 6) << " | "
                     << cellToChar(board[7], 7) << " | "
                     << cellToChar(board[8], 8) << " \n";
    std::cout << "\n";
}

std::string playerToString(Player p) {
    switch (p) {
        case Player::one: return "Player 1 (X)";
        case Player::two: return "Player 2 (O)";
        default: return "None";
    }
}

void printHelp() {
    std::cout << "Commands:\n";
    std::cout << "  <0-8>  - Make a move at position (see board layout)\n";
    std::cout << "  board  - Show the board layout\n";
    std::cout << "  reset  - Start a new game\n";
    std::cout << "  help   - Show this help message\n";
    std::cout << "  quit   - Exit the game\n";
}

int main() {
    TicTacToeEngine engine;
    std::string input;

    std::cout << "=== TicTacToe REPL ===\n";
    std::cout << "Type 'help' for commands.\n";
    printBoard(engine);

    while (true) {
        if (engine.phase() == Phase::finished) {
            Player winner = engine.getWinner();
            if (winner == Player::none)
                std::cout << "Game over! It's a draw!\n";
            else
                std::cout << "Game over! " << playerToString(winner) << " wins!\n";
            std::cout << "Type 'reset' to play again or 'quit' to exit.\n";
        } else
            std::cout << playerToString(engine.currentTurn()) << "'s turn.\n";

        std::cout << "> ";
        if (!std::getline(std::cin, input))
            break;

        // Trim whitespace
        size_t start = input.find_first_not_of(" \t");
        if (start == std::string::npos) 
            continue;

        input = input.substr(start);
        size_t end = input.find_last_not_of(" \t");
        input = input.substr(0, end + 1);

        if (input == "quit" || input == "exit") {
            std::cout << "Goodbye!\n";
            break;
        } else if (input == "help")
            printHelp();
        else if (input == "board")
            printBoard(engine);
        else if (input == "reset") {
            engine = TicTacToeEngine();
            std::cout << "New game started!\n";
            printBoard(engine);
        } else {
            // Try to parse as a move
            std::istringstream iss(input);
            int position;
            if (iss >> position) {
                if (engine.phase() == Phase::finished) {
                    std::cout << "Game is over. Type 'reset' to start a new game.\n";
                    continue;
                }

                MoveResult result = engine.move(engine.currentTurn(), position);
                if (result.success)
                    printBoard(engine);
                else {
                    switch (result.error) {
                        case MoveError::outOfBounds:
                            std::cout << "Invalid position. Use 0-8.\n";
                            break;
                        case MoveError::taken:
                            std::cout << "That position is already taken.\n";
                            break;
                        case MoveError::notYourTurn:
                            std::cout << "It's not your turn.\n";
                            break;
                    }
                }
            }
            else
                std::cout << "Unknown command. Type 'help' for available commands.\n";
        }
    }

    return 0;
}