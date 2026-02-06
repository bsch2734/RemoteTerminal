#include "TicTacToeEngine.h"
#include "TicTacToeEngine.h"
#include "TicTacToeEngine.h"

using namespace TicTacToe;

TicTacToeEngine::TicTacToeEngine()
	: _phase(Phase::playing), _currentPlayer(Player::one), _board{ Player::none, Player::none, Player::none,
																   Player::none, Player::none, Player::none,
																   Player::none, Player::none, Player::none } {
}

MoveResult TicTacToeEngine::move(Player p, int target) {
	MoveResult answer;

	if (_phase == Phase::finished) {
		answer.success = false;
		answer.error = MoveError::notYourTurn;
		return answer;
	}
	if (p != _currentPlayer) {
		answer.success = false;
		answer.error = MoveError::notYourTurn;
		return answer;
	}
	if (target < 0 || target >= 9) {
		answer.success = false;
		answer.error = MoveError::outOfBounds;
		return answer;
	}
	if (_board[target] != Player::none) {
		answer.success = false;
		answer.error = MoveError::taken;
		return answer;
	}
	_board[target] = p;
	_moveCount++;
	//check for win
	for (int i = 0; i < 3; i++) {
		//rows
		if (_board[i * 3] == p && _board[i * 3 + 1] == p && _board[i * 3 + 2] == p) {
			_phase = Phase::finished;
			answer.success = true;
			return answer;
		}
		//cols
		if (_board[i] == p && _board[i + 3] == p && _board[i + 6] == p) {
			_phase = Phase::finished;
			answer.success = true;
			return answer;
		}
	}
	//diags
	if (_board[0] == p && _board[4] == p && _board[8] == p) {
		_phase = Phase::finished;
		answer.success = true;
		return answer;
	}
	if (_board[2] == p && _board[4] == p && _board[6] == p) {
		_phase = Phase::finished;
		answer.success = true;
		return answer;
	}
	//check for draw
	if (_moveCount == 9) {
		_phase = Phase::finished;
		_currentPlayer = Player::none;
		answer.success = true;
		return answer;
	}

	//switch turn
	_currentPlayer = opponent(p);
	answer.success = true;
	return answer;
}

Phase TicTacToeEngine::phase() const {
	return _phase;
}

Player TicTacToeEngine::getWinner() const {
	if (Phase::finished != _phase)
		return Player::none;
	return _currentPlayer;
}

Player TicTacToeEngine::currentTurn() const {
	return _currentPlayer;
}

const std::array<Player, 9>& TicTacToeEngine::getBoard() const {
	return _board;
}
