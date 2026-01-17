#pragma once

#include <string>
#include <json/json.h>
#include "BattleshipMessageRouter.h"
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

Json::Value toJson(Phase p);

Json::Value toJson(const std::string& s);

Json::Value toJson(const std::vector<UserView>& us);

Json::Value toJson(const UserView& u);

Json::Value toJson(const BoardView& b);

Json::Value toJson(const GridView& g);

Json::Value toJson(const SquareState& s);

Json::Value toJson(const SquareView& s);

Json::Value toJson(const SessionActionResultData& s);

Json::Value toJson(const SessionActionResultError& s);

Json::Value toJson(const FireResultData& f);

Json::Value toJson(const ReadyResultData& f);

Json::Value toJson(const PlaceShipResultData& f);

Json::Value toJson(const CheckPlacementResultData& f);

Json::Value toJson(const UserSnapshot& u);

Json::Value toJson(const StartupInfo& s);

Json::Value toJson(const Fleet& f);

Json::Value toJson(const Ship& s);

Json::Value toJson(const std::set<coord> s);

Json::Value toJson(const AddUserToGameResult& r);

Json::Value toJson(const AddUserToGameError& e);

Json::Value toJson(const OutboundMessage& r);

JoinRequest joinRequestFromJson(const Json::Value& v);

Json::Value parseJson(const std::string& s);

ActionRequest actionRequestFromJson(const Json::Value& v);

OutboundWireMessage outboundWireMessageFromJson(const Json::Value v);
