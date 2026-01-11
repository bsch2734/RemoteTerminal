#include "BattleshipSession.h"
#include "Action.h"

#include <string>

BattleshipSession::BattleshipSession(const GameId& id, const UserId& playerOneId, const UserId& playerTwoId) {
	_userToPlayerMap[playerOneId] = Player::one;
	_userToPlayerMap[playerTwoId] = Player::two;
	_playerToUserMap[Player::one] = playerOneId;
	_playerToUserMap[Player::two] = playerTwoId;
	_playerToUserMap[Player::none] = "";
	_gameId = id;
}

GameId BattleshipSession::getGameId() const {
	return _gameId;
}

bool BattleshipSession::isFinished() const {
	return _engine.phase() == Phase::finished;
}

SessionActionResult BattleshipSession::handleAction(const UserId& user, const SessionAction& action) {
	SessionActionResult s;
	Player p = playerFor(user);
	if (p == Player::none) {
		s.success = false;
		s.error = SessionActionResultError::userNotFound;
		return s;
	}

	switch (action.type) {
		case SessionActionType::PlaceShip: {
			s = handlePlaceShip(p, action);
			break;
		}
		case SessionActionType::Ready: {
			s = handleReady(p);
			break;
		}
		case SessionActionType::Fire: {
			s = handleFire(p, action);
			break;
		}
		default: {
			s.success = true;
			s.error = SessionActionResultError::unknownAction;
			break;
		}
	}

	return s;
}

UserSnapshot BattleshipSession::getSnapshotForUser(const UserId& u) {
	UserSnapshot answer;
	answer.currentUser = _playerToUserMap[_engine.currentTurn()];
	answer.phase = _engine.phase();
	answer.userView = UserView(u, _engine.boardViewForPlayer(_userToPlayerMap[u]));
	answer.youReady = _engine.isPlayerReady(_userToPlayerMap[u]);
	answer.opponentReady = _engine.isPlayerReady(_userToPlayerMap[opponentForUser(u)]);
	return answer;
}

StartupInfo BattleshipSession::getStartupInfoForUser(const UserId& u) {
	return StartupInfo(
		_engine.phase(),
		u,
		opponentForUser(u),
		_gameId,
		_engine.getFleetForPlayer(_userToPlayerMap[u]),
		UserView(u, _engine.boardViewForPlayer(_userToPlayerMap[u])),
		_engine.boardRows(),
		_engine.boardCols()	
	);
}

UserId BattleshipSession::opponentForUser(const UserId& u) {
	return _playerToUserMap[oponent(_userToPlayerMap[u])];
}

Player BattleshipSession::playerFor(const UserId& user) const {
	auto p = _userToPlayerMap.find(user);
	if (p == _userToPlayerMap.end())
		return Player::none;
	return p->second;
}

SessionActionResult BattleshipSession::handlePlaceShip(Player p, const SessionAction& a) {
	SessionActionResult answer;
	
	PlaceShipData psd = std::get<PlaceShipData>(a.data);
	auto r = _engine.placeShip(p, psd.shipId, psd.posision, psd.rotation);

	answer.success = r.success;
	answer.type = SessionActionResultType::PlaceShipResult;

	if (answer.success) {
		answer.data = PlaceShipResultData();
	}
	else {
		switch (r.error) {
			case PlaceShipError::OverlapsAnotherShip:
			case PlaceShipError::OutOfBounds : {
				answer.error = SessionActionResultError::invalidPlacement;
				break;
			}
			case PlaceShipError::WrongPhase: {
				answer.error = SessionActionResultError::wrongPhase;
				break;
			}
			case PlaceShipError::invalidID: {
				answer.error = SessionActionResultError::shipNotFound;
				break;
			}
		}
	}
	
	return answer;;
}

SessionActionResult BattleshipSession::handleFire(Player p, const SessionAction& a) {
	SessionActionResult answer;
	FireResult r = _engine.fire(p, std::get<FireData>(a.data).target);

	answer.success = r.success;
	answer.type = SessionActionResultType::FireResult;

	if (answer.success) {
		FireResultData d;
		d.isHit = r.isHit;
		d.isSunk = r.isSink;
		if (r.isSink) {
			d.sunkName = _engine.nameForId(r.hitId);
			d.hitId = r.hitId;
		}
		answer.data = d;
	}
	else {
		switch (r.error) {
			case FireError::outOfBounds: {
				answer.error = SessionActionResultError::invalidPlacement;
				break;
			}
			case FireError::notYourTurn: {
				answer.error = SessionActionResultError::notYourTurn;
				break;
			}
		}
	}

	return answer;
}

SessionActionResult BattleshipSession::handleReady(Player p) {
	SessionActionResult answer;
	ReadyUpResult r = _engine.readyUp(p);
	answer.success = r.succes;
	answer.type = SessionActionResultType::ReadyResult;
	
	if (answer.success) {
		answer.data = ReadyResultData();
	}
	else {
		switch (r.error) {
			//may want to refine these errors for clarity
			case ReadyUpError::fleetNotPlaced: {
				answer.error = SessionActionResultError::invalidPlacement;
				break;
			}
			case ReadyUpError::fleetPlacementInvalid: {
				answer.error = SessionActionResultError::notYourTurn;
				break;
			}
		}
	}

	return answer;
}
