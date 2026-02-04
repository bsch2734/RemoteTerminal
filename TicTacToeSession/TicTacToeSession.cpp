#include "TicTacToeSession.h"

TicTacToeSession::TicTacToeSession(const GameId& id, const UserId& playerOneId, const UserId& playerTwoId) {
	_userToPlayerMap[playerOneId] = Player::one;
	_userToPlayerMap[playerTwoId] = Player::two;
	_playerToUserMap[Player::one] = playerOneId;
	_playerToUserMap[Player::two] = playerTwoId;
	_playerToUserMap[Player::none] = "";
	_gameId = id;
}

GameId TicTacToeSession::getGameId() const {
	return _gameId;
}

bool TicTacToeSession::isFinished() const {
	return _engine.phase() == Phase::finished;
}

AddressedMessageBundle TicTacToeSession::handleAction(const UserId& user, const SessionAction& action) {
	AddressedMessageBundle a;
	SessionActionResult s;
	Player p = playerFor(user);
	if (p == Player::none) {
		s.success = false;
		s.error = SessionActionResultError::userNotFound;
		return a.addMessage(ToUser(user), s);
	}
	switch (action.type) {
		case SessionActionType::Move: {
			s = handleMove(p, action);
			break;
		}
		default: {
			s.success = false;
			s.error = SessionActionResultError::unknownAction;
			break;
		}
	}
	a.addMessage(ToUser(user), s);
	a.addMessageBundle(getSnapshotMessageBundles());
	return a;
}

AddressedMessageBundle TicTacToeSession::getSnapshotMessageBundles() {
	AddressedMessageBundle answer;
	UserSnapshot snapshot;
	snapshot.currentUser = _playerToUserMap[_engine.currentTurn()];
	snapshot.phase = _engine.phase();
	snapshot.board = _engine.getBoard();
	answer.addMessage(ToUser(_playerToUserMap[Player::one]), snapshot);
	answer.addMessage(ToUser(_playerToUserMap[Player::two]), snapshot);
	return answer;
}

UserId TicTacToeSession::opponentForUser(const UserId& u) {
	return _playerToUserMap[opponent(_userToPlayerMap[u])];
}

Player TicTacToeSession::playerFor(const UserId& user) const {
	auto p = _userToPlayerMap.find(user);
	if (p == _userToPlayerMap.end())
		return Player::none;
	return p->second;
}

SessionActionResult TicTacToeSession::handleMove(Player p, const SessionAction& a) {
	SessionActionResult answer;
	MoveData md = std::get<MoveData>(a.data); //?
	auto r = _engine.move(p, md.target);
	answer.success = r.success;
	answer.type = SessionActionResultType::MoveResult;
	if (answer.success) {
		answer.data = MoveResultData(); //?
	}
	else {
		switch (r.error) {
			case MoveError::outOfBounds: {
				answer.error = SessionActionResultError::invalidPlacement;
				break;
			}
			case MoveError::taken: {
				answer.error = SessionActionResultError::cellTaken;
				break;
			}
			case MoveError::notYourTurn: {
				answer.error = SessionActionResultError::notYourTurn;
				break;
			}
		}
	}
	return answer;
}



