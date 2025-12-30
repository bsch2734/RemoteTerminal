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
		s.error = SessionError::userNotFound;
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
			s.error = SessionError::unknownAction;
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
	PlaceShipData psd = std::get<PlaceShipData>(a.data);
	auto r = _engine.placeShip(p, psd.shipId, psd.posision, psd.rotation);
	return SessionResult();
}

SessionResult BattleshipSession::handleFire(Player p, const Action& a) {
	auto r = _engine.fire(p, std::get<FireData>(a.data).target);
	return SessionResult();
}

SessionResult BattleshipSession::handleReady(Player p) {
	auto r = _engine.readyUp(p);
	return SessionResult();
}
