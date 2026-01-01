#include "BattleshipSession.h"
#include "Action.h"

#include <string>

BattleshipSession::BattleshipSession(std::string id, std::string playerOneId, std::string playerTwoId) {
	_userToPlyaerMap[playerOneId] = Player::one;
	_userToPlyaerMap[playerTwoId] = Player::two;
}

std::string BattleshipSession::id() const {
	return _sessionId;
}

bool BattleshipSession::isFinished() const {
	return _engine.phase() == Phase::finished;
}

SessionResult BattleshipSession::handleAction(std::string user, const Action& action) {
	SessionResult s;
	Player p = playerFor(user);
	if (p == Player::none) {
		s.success = false;
		s.error = SessionResultError::userNotFound;
		return s;
	}

	switch (action.type) {
		case ActionType::PlaceShip: {
			s = handlePlaceShip(p, action);
			break;
		}
		case ActionType::Ready: {
			s = handleReady(p);
			break;
		}
		case ActionType::Fire: {
			s = handleFire(p, action);
			break;
		}
		default: {
			s.success = true;
			s.error = SessionResultError::unknownAction;
			break;
		}
	}

	return s;
}

Player BattleshipSession::playerFor(std::string user) const {
	auto p = _userToPlyaerMap.find(user);
	if (p == _userToPlyaerMap.end())
		return Player::none;
	return p->second;
}

SessionResult BattleshipSession::handlePlaceShip(Player p, const Action& a) {
	SessionResult answer;
	
	PlaceShipData psd = std::get<PlaceShipData>(a.data);
	auto r = _engine.placeShip(p, psd.shipId, psd.posision, psd.rotation);

	answer.success = r.success;
	answer.type = SessionResultType::PlaceShipResult;

	if (answer.success) {
		answer.data = PlaceShipResultData();
	}
	else {
		switch (r.error) {
			case PlaceShipError::OverlapsAnotherShip:
			case PlaceShipError::OutOfBounds : {
				answer.error = SessionResultError::invalidPlacement;
				break;
			}
			case PlaceShipError::WrongPhase: {
				answer.error = SessionResultError::wrongPhase;
				break;
			}
			case PlaceShipError::invalidID: {
				answer.error = SessionResultError::shipNotFound;
				break;
			}
		}
	}
	
	return answer;;
}

SessionResult BattleshipSession::handleFire(Player p, const Action& a) {
	SessionResult answer;
	FireResult r = _engine.fire(p, std::get<FireData>(a.data).target);

	answer.success = r.success;
	answer.type = SessionResultType::FireResult;

	if (answer.success) {
		FireResultData d;
		d.isHit = r.isHit;
		d.hitId = r.hitId;
		d.isSunk = r.isSink;
		d.sunkName = _engine.nameForId(r.hitId);
		answer.data = d;
	}
	else {
		switch (r.error) {
			case FireError::outOfBounds: {
				answer.error = SessionResultError::invalidPlacement;
				break;
			}
			case FireError::notYourTurn: {
				answer.error = SessionResultError::notYourTurn;
				break;
			}
		}
	}

	return answer;
}

SessionResult BattleshipSession::handleReady(Player p) {
	SessionResult answer;
	ReadyUpResult r = _engine.readyUp(p);
	answer.success = r.succes;
	answer.type = SessionResultType::ReadyResult;
	
	if (answer.success) {
		answer.data = ReadyResultData();
	}
	else {
		switch (r.error) {
			//may want to refine these errors for clarity
			case ReadyUpError::fleetNotPlaced: {
				answer.error = SessionResultError::invalidPlacement;
				break;
			}
			case ReadyUpError::fleetPlacementInvalid: {
				answer.error = SessionResultError::notYourTurn;
				break;
			}
		}
	}

	return answer;
}
