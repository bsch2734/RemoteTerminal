#pragma once

#include <string>
#include <json/json.h>
#include "EndpointTypes.h"
#include "SessionTypes.h"

namespace TicTacToe {

Json::Value toJson(const SessionAction& a);
SessionAction sessionActionFromJson(const Json::Value& v);

Json::Value toJson(const MoveData& d);
MoveData moveDataFromJson(const Json::Value& v);

Json::Value toJson(const SessionActionType& t);
SessionActionType sessionActionTypeFromJson(const Json::Value& v);

Json::Value toJson(const SessionActionData& d);

GameId gameIdFromJson(const Json::Value& v);

UserId userIdFromJson(const Json::Value& v);

Json::Value toJson(const SessionActionResultType& r);

Json::Value toJson(const SessionActionResult& r);

Json::Value toJson(Phase p);

Json::Value toJson(const std::string& s);

Json::Value toJson(const Player& p);

Json::Value toJson(const std::array<Player, 9>& board);

Json::Value toJson(const SessionActionResultData& s);

Json::Value toJson(const SessionActionResultError& s);

Json::Value toJson(const MoveResultData& m);

Json::Value toJson(const UserSnapshot& u);

Json::Value toJson(const StartupInfo& s);

Json::Value toJson(const AddUserToGameResult& r);

Json::Value toJson(const AddUserToGameError& e);

Json::Value toJson(const OutboundMessage& r);

JoinRequest joinRequestFromJson(const Json::Value& v);

Json::Value parseJson(std::string_view s);

ActionRequest actionRequestFromJson(const Json::Value& v);

OutboundWireMessage outboundWireMessageFromJson(const Json::Value v);

} // namespace TicTacToe
