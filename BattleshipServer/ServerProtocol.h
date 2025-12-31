#pragma once

#include <string>
#include <json/json.h>
#include "GameEntities.h"
#include "coord.h"
#include "Action.h"

Json::Value toJson(const coord& c);
coord coordFromJson(const Json::Value& v);

Json::Value toJson(const Action& a);
Action actionFromJson(const Json::Value& v);

Json::Value toJson(const FireData& d);
FireData fireDataFromJson(const Json::Value& v);

Json::Value toJson(const ReadyData& d);
ReadyData readyDataFromJson(const Json::Value& v);

Json::Value toJson(const PlaceShipData& d);
PlaceShipData placeShipDataFromJson(const Json::Value& v);

Json::Value toJson(const ActionData& d);

std::string gameIdFromJson(const Json::Value& v);

std::string userIdFromJson(const Json::Value& v);