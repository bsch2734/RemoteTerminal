#include "TicTacToeSession.h"

using namespace TicTacToe;

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
		case SessionActionType::Rematch: {
			s = handleRematch(p);
			return processRematchRequest(user, p, s);
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

AddressedMessageBundle TicTacToeSession::getStartupInfoBundles() {
	AddressedMessageBundle answer;

	UserId playerOne = _playerToUserMap[Player::one];
	UserId playerTwo = _playerToUserMap[Player::two];

	StartupInfo infoForPlayerOne;
	infoForPlayerOne.you = playerOne;
	infoForPlayerOne.opponent = playerTwo;
	infoForPlayerOne.yourSymbol = Player::one;
	infoForPlayerOne.opponentSymbol = Player::two;

	StartupInfo infoForPlayerTwo;
	infoForPlayerTwo.you = playerTwo;
	infoForPlayerTwo.opponent = playerOne;
	infoForPlayerTwo.yourSymbol = Player::two;
	infoForPlayerTwo.opponentSymbol = Player::one;

	answer.addMessage(ToUser(playerOne), infoForPlayerOne);
	answer.addMessage(ToUser(playerTwo), infoForPlayerTwo);

	return answer;
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

SessionActionResult TicTacToeSession::handleRematch(Player p) {
	SessionActionResult answer;
	answer.type = SessionActionResultType::RematchResult;
	answer.data = RematchResultData{};
	
	// Guard: Only allow rematch when game is finished
	if (_engine.phase() != Phase::finished) {
		answer.success = false;
		answer.error = SessionActionResultError::wrongPhase;
		return answer;
	}
	
	answer.success = true;
	
	// Mark this player as wanting a rematch
	if (p == Player::one) {
		_playerOneWantsRematch = true;
	} else if (p == Player::two) {
		_playerTwoWantsRematch = true;
	}
	
	return answer;
}

AddressedMessageBundle TicTacToeSession::processRematchRequest(const UserId& user, Player p, const SessionActionResult& result) {
	AddressedMessageBundle a;
	
	// If rematch request failed, just send error to user
	if (!result.success) {
		a.addMessage(ToUser(user), result);
		return a;
	}
	
	// Check if both players want a rematch
	if (_playerOneWantsRematch && _playerTwoWantsRematch) {
		// Reset the engine for a new game
		_engine = TicTacToeEngine();
		_playerOneWantsRematch = false;
		_playerTwoWantsRematch = false;
		
		// Send rematch start to both players
		RematchStart rematchStart{};
		a.addMessage(ToUser(_playerToUserMap[Player::one]), rematchStart);
		a.addMessage(ToUser(_playerToUserMap[Player::two]), rematchStart);
		// Send new startup info to both players
		a.addMessageBundle(getStartupInfoBundles());
		a.addMessageBundle(getSnapshotMessageBundles());
		return a;
	}
	
	// Send rematch request to opponent
	UserId opponent = opponentForUser(user);
	a.addMessage(ToUser(opponent), RematchRequest{user});
	a.addMessage(ToUser(user), result);
	return a;
}
