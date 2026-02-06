#include "BattleshipJsonProtocol.h"

// Battleship namespace functions
namespace Battleship {

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
	else if (t == SessionActionType::CheckPlacement)
		d = placeShipDataFromJson(actionDataJson); // Uses same data format as PlaceShip
	else if (t == SessionActionType::Rematch)
		d = rematchDataFromJson(actionDataJson);

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

Json::Value toJson(const RematchData& d) {
	return Json::nullValue;
}

RematchData rematchDataFromJson(const Json::Value& v) {
	return RematchData();
}

Json::Value toJson(const PlaceShipData& d) {
	Json::Value answer(Json::objectValue);
	answer["position"] = toJson(d.position);
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
		case SessionActionType::CheckPlacement: {
			answer = "checkplacement";
			break;
		}
		case SessionActionType::Rematch: {
			answer = "rematch";
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
	if (s == "checkplacement")
		answer = SessionActionType::CheckPlacement;
	if (s == "rematch")
		answer = SessionActionType::Rematch;

	return answer;
}

Json::Value toJson(const SessionActionData& d) {
	if (std::holds_alternative<FireData>(d))
		return toJson(std::get<FireData>(d));
	if (std::holds_alternative<ReadyData>(d))
		return toJson(std::get<ReadyData>(d));
	if (std::holds_alternative<PlaceShipData>(d))
		return toJson(std::get<PlaceShipData>(d));
	if (std::holds_alternative<RematchData>(d))
		return toJson(std::get<RematchData>(d));
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
		case SessionActionResultType::CheckPlacementResult: {
			answer = "checkplacementresult";
			break;
		}
		case SessionActionResultType::RematchResult: {
			answer = "rematchresult";
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
	answer["opponentgrid"] = toJson(b.opponentGrid);
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
	else if (std::holds_alternative<CheckPlacementResultData>(s))
		answer = toJson(std::get<CheckPlacementResultData>(s));
	else if (std::holds_alternative<RematchResultData>(s))
		answer = toJson(std::get<RematchResultData>(s));
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

Json::Value toJson(const RematchResultData& r) {
	return Json::Value(Json::nullValue);
}

Json::Value toJson(const CheckPlacementResultData& c) {
	Json::Value answer(Json::objectValue);
	answer["valid"] = c.valid;
	answer["coords"] = toJson(c.coords);
	return answer;
}

Json::Value toJson(const RematchRequest& r) {
	Json::Value answer(Json::objectValue);
	answer["user"] = toJson(r.requestingUser);
	return answer;
}

Json::Value toJson(const RematchStart& r) {
	return Json::Value(Json::objectValue);
}

Json::Value toJson(const StartupInfo& s) {
	Json::Value answer(Json::objectValue);
	answer["phase"] = toJson(s.phase);
	answer["you"] = toJson(s.you);
	answer["opponent"] = toJson(s.opponent);
	answer["gameid"] = toJson(s.gameId);
	answer["userview"] = toJson(s.userView);
	answer["fleet"] = toJson(s.fleet);
	answer["boardrows"] = s.boardRows;
	answer["boardcols"] = s.boardCols;
	return answer;
}

Json::Value toJson(const UserSnapshot& u) {
	Json::Value answer(Json::objectValue);
	answer["phase"] = toJson(u.phase);
	answer["currentturn"] = toJson(u.currentUser);
	answer["userview"] = toJson(u.userView);
	answer["youready"] = u.youReady;
	answer["opponentready"] = u.opponentReady;
	return answer;
}

Json::Value toJson(const Fleet& f) {
	Json::Value answer(Json::arrayValue);
	for (const Ship& s : f.getShips())
		answer.append(toJson(s));
	return answer;
}

Json::Value toJson(const Ship& s) {
	Json::Value answer(Json::objectValue);
	answer["name"] = toJson(s.getName());
	answer["id"] = s.getID();
	answer["coords"] = toJson(s.getCoords());
	return answer;
}

Json::Value toJson(const std::set<coord> s) {
	Json::Value answer(Json::arrayValue);
	for (const coord& c : s)
		answer.append(toJson(c));
	return answer;
}

Json::Value toJson(const AddUserToGameResult& r) {
	Json::Value answer(Json::objectValue);

	answer["success"] = r.success;
	answer["readytostart"] = r.readyToStart;
	answer["error"] = toJson(r.error);

	if (!r.readyToStart)
		answer["waiting"] = true; // backwards compatability for now, fix to be more in line with rest of code

	return answer;
}

Json::Value toJson(const AddUserToGameError& e) {
	Json::Value answer(Json::stringValue);
	switch (e) {
		case AddUserToGameError::userAlreadyInGame: {
			answer = "useralreadyingame";
			break;
		}
		case AddUserToGameError::gameFull: {
			answer = "gamefull";
			break;
		}
	}
	return answer;
}

Json::Value toJson(const OutboundMessage& r) {
	Json::Value answer(Json::objectValue);

	if (std::holds_alternative<UserSnapshot>(r))
		answer["snapshot"] = toJson(std::get<UserSnapshot>(r));
	else if (std::holds_alternative<StartupInfo>(r))
		answer["setupinfo"] = toJson(std::get<StartupInfo>(r));
	else if (std::holds_alternative<SessionActionResult>(r))
		answer["actionresult"] = toJson(std::get<SessionActionResult>(r));
	else if (std::holds_alternative<AddUserToGameResult>(r))
		answer = toJson(std::get<AddUserToGameResult>(r));
	else if (std::holds_alternative<RematchRequest>(r))
		answer["rematchrequest"] = toJson(std::get<RematchRequest>(r));
	else if (std::holds_alternative<RematchStart>(r))
		answer["rematchstart"] = toJson(std::get<RematchStart>(r));

	return answer;
}

JoinRequest joinRequestFromJson(const Json::Value& v) {
	JoinRequest answer(
		userIdFromJson(v["userid"]),
		gameIdFromJson(v["gameid"])
	);
	return answer;
}

Json::Value parseJson(std::string_view s)
{
	Json::CharReaderBuilder rb;
	Json::Value root;
	std::string errs;

	std::unique_ptr<Json::CharReader> reader(rb.newCharReader());

	const char* begin = s.data();
	const char* end = s.data() + s.size();

	if (!reader->parse(begin, end, &root, &errs))
		return Json::nullValue;

	return root;
}

ActionRequest actionRequestFromJson(const Json::Value& v) {
	ActionRequest answer(
		gameIdFromJson(v["gameid"]),
		userIdFromJson(v["userid"]),
		sessionActionFromJson(v["sessionaction"])
	);
	return answer;
}

OutboundWireMessage outboundWireMessageFromJson(const Json::Value v) {
	Json::StreamWriterBuilder wb;
	wb["indentation"] = ""; // single-line

	return Json::writeString(wb, v);
}

} // namespace Battleship
