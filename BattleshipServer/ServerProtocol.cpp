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

Json::Value toJson(const Action& a) {
	Json::Value answer(Json::objectValue);
	std::string actionType;
	switch (a.type) {
		case ActionType::PlaceShip: {
			actionType = "PlaceShip";
			break;
		}
		case ActionType::Ready: {
			actionType = "Ready";
			break;
		}
		case ActionType::Fire: {
			actionType = "Fire";
			break;
		}
	}
	answer["type"] = actionType;
	answer["data"] = toJson(a.data);
	return answer;
}

Action actionFromJson(const Json::Value& v) {
	std::string s = v["type"].asString();
	
	ActionType t{};
	ActionData d = ReadyData();

	Json::Value actionDataJson = v["data"];

	if (s == "PlaceShip") {
		t = ActionType::PlaceShip;
		d = placeShipDataFromJson(actionDataJson);
	}
	else if (s == "Read") {
		t = ActionType::Ready;
		d = readyDataFromJson(actionDataJson);
	}
	else if (s == "Fire") {
		t = ActionType::Fire;
		d = fireDataFromJson(actionDataJson);
	}

	return Action(t, d);
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

Json::Value toJson(const ActionData& d) {
	if (std::holds_alternative<FireData>(d))
		return toJson(std::get<FireData>(d));
	if (std::holds_alternative<ReadyData>(d))
		return toJson(std::get<ReadyData>(d));
	if (std::holds_alternative<PlaceShipData>(d))
		return toJson(std::get<PlaceShipData>(d));
	return Json::nullValue;
}

std::string gameIdFromJson(const Json::Value& v) {
	return v["gameId"].asString();
}

std::string userIdFromJson(const Json::Value& v) {
	return v["userId"].asString();
}
