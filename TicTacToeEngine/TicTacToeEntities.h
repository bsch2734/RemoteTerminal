#pragma once

namespace TicTacToe {

enum class Player {
	none = 0,
	one = 1,
	two = 2
};

inline Player opponent(Player p) {
	if (p == Player::none)
		return Player::none;
	return (p == Player::one) ? Player::two : Player::one;
}

enum class Phase {
	playing,
	finished
};

enum class MoveError {
	outOfBounds,
	taken,
	notYourTurn
};

struct MoveResult {
	bool success = false;
	MoveError error = MoveError::notYourTurn;
};

} // namespace TicTacToe
