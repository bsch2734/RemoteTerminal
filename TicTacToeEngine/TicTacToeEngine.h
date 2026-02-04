#pragma once

#include "TicTacToeEntities.h"

#include <array>

namespace TicTacToe {

class TicTacToeEngine {
public:
    TicTacToeEngine();

    MoveResult move(Player p, int target);

    Phase phase() const;
    Player getWinner() const;
    Player currentTurn() const;
    const std::array<Player, 9>& getBoard() const;

private:
    int _moveCount = 0;

    Phase _phase;

    Player _currentPlayer;

    std::array<Player, 9> _board;
};

} // namespace TicTacToe
