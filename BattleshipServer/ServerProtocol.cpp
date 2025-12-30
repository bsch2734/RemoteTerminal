#include "ServerProtocol.h"


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