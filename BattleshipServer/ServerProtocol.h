#pragma once

#include <string>
#include <json/json.h>
#include "GameEntities.h"
#include "coord.h"

Json::Value toJson(const coord& c);
coord coordFromJson(const Json::Value& v);