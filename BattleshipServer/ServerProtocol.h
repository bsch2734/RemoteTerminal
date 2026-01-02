#pragma once

#include <string>
#include <json/json.h>
#include "GameEntities.h"
#include "coord.h"
#include "Action.h"
#include "SessionTypes.h"

Json::Value toJson(const coord& c);
coord coordFromJson(const Json::Value& v);

Json::Value toJson(const SessionAction& a);
SessionAction sessionActionFromJson(const Json::Value& v);

Json::Value toJson(const FireData& d);
FireData fireDataFromJson(const Json::Value& v);

Json::Value toJson(const ReadyData& d);
ReadyData readyDataFromJson(const Json::Value& v);

Json::Value toJson(const PlaceShipData& d);
PlaceShipData placeShipDataFromJson(const Json::Value& v);

Json::Value toJson(const SessionActionType& t);
SessionActionType sessionActionTypeFromJson(const Json::Value& v);

Json::Value toJson(const SessionActionData& d);

UserId gameIdFromJson(const Json::Value& v);

GameId userIdFromJson(const Json::Value& v);

Json::Value toJson(const SessionActionResultType& r);

Json::Value toJson(const SessionActionResult& r);

Json::Value toJson(const SessionSnapshot& r);

Json::Value toJson(Phase p);

struct ServerUpdate {
	SessionActionResult result;
	SessionSnapshot snapshot;
};

Json::Value toJson(ServerUpdate s);

Json::Value toJson(const std::string& s);

Json::Value toJson(const std::vector<UserView>& us);

Json::Value toJson(const UserView& u);

Json::Value toJson(const BoardView& b);

Json::Value toJson(const GridView& g);

Json::Value toJson(const SquareState& s);

Json::Value toJson(const SquareView& s);