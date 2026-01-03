#include "BattleshipJsonProtocol.h"


Json::Value toJson(const coord& c) {
	if (c.isUnspecified())
		return Json::nullValue;

	Json::Value answer(Json::objectValue);
	answer["row"] = c.d;
	answer["col"] = c.o;
	return answer;
}

coord coordFromJson(const Json::Value& v) {
	if (v.isNull())
		return coord::unspecified;
	return coord({v["row"].asInt(), v["col"].asInt()});
}

Json::Value toJson(const SessionAction& a) {
	Json::Value answer(Json::objectValue);
	answer["type"] = toJson(a.type);
	answer["data"] = toJson(a.data);
	return answer;
}

SessionAction sessionActionFromJson(const Json::Value& v) {
	SessionActionType t = sessionActionTypeFromJson(v["type"]);
	
	SessionActionData d = ReadyData();

	Json::Value actionDataJson = v["data"];

	if (t == SessionActionType::PlaceShip)
		d = placeShipDataFromJson(actionDataJson);
	else if (t == SessionActionType::Ready)
		d = readyDataFromJson(actionDataJson);
	else if (t == SessionActionType::Fire)
		d = fireDataFromJson(actionDataJson);

	return SessionAction(t, d);
}

Json::Value toJson(const FireData& d) {
	Json::Value answer(Json::objectValue);
	answer["target"] = toJson(d.target);
	return answer;
}

FireData fireDataFromJson(const Json::Value& v) {
	return FireData(coordFromJson(v["target"]));
}

Json::Value toJson(const ReadyData& d) {
	return Json::nullValue;
}

ReadyData readyDataFromJson(const Json::Value& v) {
	return ReadyData();
}

Json::Value toJson(const PlaceShipData& d) {
	Json::Value answer(Json::objectValue);
	answer["position"] = toJson(d.posision);
	answer["rotation"] = d.rotation;
	answer["shipid"] = d.shipId;
	return answer;
}

PlaceShipData placeShipDataFromJson(const Json::Value& v) {
	coord c = coordFromJson(v["position"]);
	int r = v["rotation"].asInt();
	int i = v["shipid"].asInt();
	return PlaceShipData(i, r, c);
}

Json::Value toJson(const SessionActionType& t) {
	Json::Value answer(Json::stringValue);
	switch (t) {
		case SessionActionType::PlaceShip: {
			answer = "placeship";
			break;
		}
		case SessionActionType::Ready: {
			answer = "ready";
			break;
		}
		case SessionActionType::Fire: {
			answer = "fire";
			break;
		}
	}
	return answer;
}

SessionActionType sessionActionTypeFromJson(const Json::Value& v) {
	SessionActionType answer = SessionActionType::Ready;

	std::string s = v.asString();
	if (s == "placeship")
		answer = SessionActionType::PlaceShip;
	if (s == "ready")
		answer = SessionActionType::Ready;
	if (s == "fire")
		answer = SessionActionType::Fire;

	return answer;
}

Json::Value toJson(const SessionActionData& d) {
	if (std::holds_alternative<FireData>(d))
		return toJson(std::get<FireData>(d));
	if (std::holds_alternative<ReadyData>(d))
		return toJson(std::get<ReadyData>(d));
	if (std::holds_alternative<PlaceShipData>(d))
		return toJson(std::get<PlaceShipData>(d));
	return Json::nullValue;
}

GameId gameIdFromJson(const Json::Value& v) {
	return v.asString();
}

UserId userIdFromJson(const Json::Value& v) {
	return v.asString();
}

Json::Value toJson(const SessionActionResultType& r) {
	Json::Value answer(Json::stringValue);

	switch (r) {
		case SessionActionResultType::PlaceShipResult: {
			answer = "placeshipresult";
			break;
		}
		case SessionActionResultType::ReadyResult: {
			answer = "readyresult";
			break;
		}
		case SessionActionResultType::FireResult: {
			answer = "fireresult";
			break;
		}
	}

	return answer;	
}

Json::Value toJson(const SessionActionResult& r) {
	Json::Value answer(Json::objectValue);

	answer["success"] = r.success;
	answer["error"] = toJson(r.error);
	answer["type"] = toJson(r.type);
	answer["data"] = toJson(r.data);

	return answer;
}

Json::Value toJson(const SessionSnapshot& r) {
	Json::Value answer(Json::objectValue);
	answer["phase"] = toJson(r.phase);
	answer["currentturn"] = toJson(r.currentUser);
	answer["userviews"] = toJson(r.userViews);
	return answer;
}

Json::Value toJson(Phase p) {
	Json::Value answer(Json::stringValue);
	switch (p) {
		case Phase::setup: {
			answer = "setup";
			break;
		}
		case Phase::playing: {
			answer = "playing";
			break;
		}
		case Phase::finished: {
			answer = "finished";
			break;
		}
	}
	return answer;
}

Json::Value toJson(ServerUpdate s) {
	Json::Value answer(Json::objectValue);

	answer["result"] = toJson(s.result);
	answer["snapshot"] = toJson(s.snapshot);

	return answer;
}

Json::Value toJson(const std::string& u) {
	Json::Value answer(Json::stringValue);
	answer = u;
	return answer;
}

Json::Value toJson(const std::vector<UserView>& us) {
	Json::Value answer(Json::arrayValue);

	for (UserView u : us)
		answer.append(toJson(u));

	return answer;
}

Json::Value toJson(const UserView& u) {
	Json::Value answer(Json::objectValue);
	answer["userid"] = toJson(u.userId);
	answer["boardview"] = toJson(u.boardView);
	return answer;
}

Json::Value toJson(const BoardView& b) {
	Json::Value answer(Json::objectValue);
	answer["owngrid"] = toJson(b.ownGrid);
	answer["opponentgrid"] = toJson(b.oponentGrid);
	return answer;
}

Json::Value toJson(const GridView& g) {
	Json::Value answer(Json::arrayValue);
	for (SquareView s : g)
		answer.append(toJson(s));
	return answer;
}

Json::Value toJson(const SquareState& s) {
	Json::Value answer(Json::stringValue);
	switch (s) {
		case SquareState::miss: {
			answer = "miss";
			break;
		}
		case SquareState::hit: {
			answer = "hit";
			break;
		}
		case SquareState::ship: {
			answer = "ship";
			break;
		}
	}
	return answer;
}

Json::Value toJson(const SquareView& s) {
	Json::Value answer;

	answer["coord"] = toJson(s.first);
	answer["state"] = toJson(s.second);

	return answer;
}

Json::Value toJson(const SessionActionResultData& s) {
	Json::Value answer(Json::objectValue);
	if (std::holds_alternative<FireResultData>(s))
		answer = toJson(std::get<FireResultData>(s));
	else if (std::holds_alternative<ReadyResultData>(s))
		answer = toJson(std::get<ReadyResultData>(s));
	else if (std::holds_alternative<PlaceShipResultData>(s))
		answer = toJson(std::get<PlaceShipResultData>(s));
	return answer;
}

Json::Value toJson(const SessionActionResultError& s) {
	Json::Value answer(Json::objectValue);
	switch (s) {
		case SessionActionResultError::internalError: {
			answer = "internalerror";
			break;
		}
		case SessionActionResultError::invalidPlacement: {
			answer = "invalidplacement";
			break;
		}
		case SessionActionResultError::notYourTurn: {
			answer = "notyourturn";
			break;
		}
		case SessionActionResultError::shipNotFound: {
			answer = "shipnotfound";
			break;
		}
		case SessionActionResultError::unknownAction: {
			answer = "unknownaction";
			break;
		}
		case SessionActionResultError::userNotFound: {
			answer = "usernotfound";
			break;
		}
		case SessionActionResultError::wrongPhase: {
			answer = "wrongphase";
			break;
		}
	}
	return answer;
}

Json::Value toJson(const FireResultData& f) {
	Json::Value answer(Json::objectValue);

	answer["ishit"] = f.isHit;
	answer["issunk"] = f.isSunk;
	answer["hitid"] = f.hitId;
	answer["sunkname"] = f.sunkName;

	return answer;
}

Json::Value toJson(const ReadyResultData& r) {
	return Json::Value(Json::nullValue);
}

Json::Value toJson(const PlaceShipResultData& p) {
	return Json::Value(Json::nullValue);
}
