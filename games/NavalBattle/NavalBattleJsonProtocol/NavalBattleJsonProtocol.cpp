#include "NavalBattleJsonProtocol.h"

// NavalBattle namespace functions
namespace NavalBattle {

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
	else if (t == SessionActionType::PlacePlane)
		d = placePlaneDataFromJson(actionDataJson);
	else if (t == SessionActionType::Ready)
		d = readyDataFromJson(actionDataJson);
	else if (t == SessionActionType::Fire)
		d = fireDataFromJson(actionDataJson);
	else if (t == SessionActionType::FireAntiAircraft)
		d = fireAntiAircraftDataFromJson(actionDataJson);
	else if (t == SessionActionType::CheckPlacement)
		d = placeShipDataFromJson(actionDataJson); // Uses same data format as PlaceShip
	else if (t == SessionActionType::CheckPlanePlacement)
		d = placePlaneDataFromJson(actionDataJson); // Uses same data format as PlacePlane
	else if (t == SessionActionType::CheckAbility)
		d = checkAbilityDataFromJson(actionDataJson);
	else if (t == SessionActionType::Rematch)
		d = rematchDataFromJson(actionDataJson);
	else if (t == SessionActionType::ActivateAbility)
		d = activateAbilityDataFromJson(actionDataJson);

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

Json::Value toJson(const FireAntiAircraftData& d) {
	Json::Value answer(Json::objectValue);
	answer["target"] = toJson(d.target);
	return answer;
}

FireAntiAircraftData fireAntiAircraftDataFromJson(const Json::Value& v) {
	return FireAntiAircraftData{coordFromJson(v["target"])};
}

Json::Value toJson(const ActivateAbilityData& d) {
	Json::Value answer(Json::objectValue);
	answer["vehicleid"] = d.vehicleId;
	answer["abilityaction"] = toJson(d.abilityAction);
	return answer;
}

ActivateAbilityData activateAbilityDataFromJson(const Json::Value& v) {
	return ActivateAbilityData{
		v["vehicleid"].asInt(),
		vehicleAbilityActionFromJson(v["abilityaction"])
	};
}

Json::Value toJson(const VehicleAbilityType& t) {
	switch (t) {
		case VehicleAbilityType::Torpedo: return "torpedo";
		case VehicleAbilityType::Exocet: return "exocet";
		case VehicleAbilityType::Apache: return "apache";
		case VehicleAbilityType::Tomahawk: return "tomahawk";
		case VehicleAbilityType::relocate: return "relocate";
		case VehicleAbilityType::scan: return "scan";
		case VehicleAbilityType::reveal: return "reveal";
	}
	return "unknown";
}

VehicleAbilityType vehicleAbilityTypeFromJson(const Json::Value& v) {
	std::string s = v.asString();
	if (s == "torpedo") return VehicleAbilityType::Torpedo;
	if (s == "exocet") return VehicleAbilityType::Exocet;
	if (s == "apache") return VehicleAbilityType::Apache;
	if (s == "tomahawk") return VehicleAbilityType::Tomahawk;
	if (s == "relocate") return VehicleAbilityType::relocate;
	if (s == "scan") return VehicleAbilityType::scan;
	if (s == "reveal") return VehicleAbilityType::reveal;
	return VehicleAbilityType::Torpedo;
}

Json::Value toJson(const VehicleAbilityUsagePolicy& p) {
	switch (p) {
		case VehicleAbilityUsagePolicy::unlimited: return "unlimited";
		case VehicleAbilityUsagePolicy::limited: return "limited";
	}
	return "unknown";
}

Json::Value toJson(const VehicleAbility& a) {
	Json::Value answer(Json::objectValue);
	answer["type"] = toJson(a.getType());
	answer["usagepolicy"] = toJson(a.getUsagePolicy());
	answer["remaininguses"] = a.getRemainingUses();
	answer["canuse"] = a.canUse();
	return answer;
}

Json::Value toJson(const Plane& p) {
	Json::Value answer(Json::objectValue);
	answer["name"] = toJson(p.getName());
	answer["id"] = p.getId();
	answer["isdestroyed"] = p.isDestroyed();
	answer["isoncarrier"] = p.isOnCarrier();

	Json::Value abilities(Json::arrayValue);
	for (const VehicleAbility& a : p.getAbilities())
		abilities.append(toJson(a));
	answer["abilities"] = abilities;

	return answer;
}

Json::Value toJson(const TorpedoData::FiringPattern& p) {
	switch (p) {
		case TorpedoData::FiringPattern::vertical: return "vertical";
		case TorpedoData::FiringPattern::horizontal: return "horizontal";
	}
	return "unknown";
}

TorpedoData::FiringPattern torpedoFiringPatternFromJson(const Json::Value& v) {
	std::string s = v.asString();
	if (s == "vertical") return TorpedoData::FiringPattern::vertical;
	if (s == "horizontal") return TorpedoData::FiringPattern::horizontal;
	return TorpedoData::FiringPattern::vertical;
}

Json::Value toJson(const ApacheData::FiringPattern& p) {
	switch (p) {
		case ApacheData::FiringPattern::vertical: return "vertical";
		case ApacheData::FiringPattern::horizontal: return "horizontal";
	}
	return "unknown";
}

ApacheData::FiringPattern apacheFiringPatternFromJson(const Json::Value& v) {
	std::string s = v.asString();
	if (s == "vertical") return ApacheData::FiringPattern::vertical;
	if (s == "horizontal") return ApacheData::FiringPattern::horizontal;
	return ApacheData::FiringPattern::vertical;
}

Json::Value toJson(const ExocetData::FiringPattern& p) {
	switch (p) {
		case ExocetData::FiringPattern::plus: return "plus";
		case ExocetData::FiringPattern::x: return "x";
	}
	return "unknown";
}

ExocetData::FiringPattern exocetFiringPatternFromJson(const Json::Value& v) {
	std::string s = v.asString();
	if (s == "plus") return ExocetData::FiringPattern::plus;
	if (s == "x") return ExocetData::FiringPattern::x;
	return ExocetData::FiringPattern::plus;
}

Json::Value toJson(const RevealData::FiringPattern& p) {
	switch (p) {
		case RevealData::FiringPattern::square: return "square";
		case RevealData::FiringPattern::diamond: return "diamond";
	}
	return "unknown";
}

RevealData::FiringPattern revealFiringPatternFromJson(const Json::Value& v) {
	std::string s = v.asString();
	if (s == "square") return RevealData::FiringPattern::square;
	if (s == "diamond") return RevealData::FiringPattern::diamond;
	return RevealData::FiringPattern::square;
}

Json::Value toJson(const TorpedoData& d) {
	Json::Value answer(Json::objectValue);
	answer["firingpattern"] = toJson(d.firingPattern);
	answer["startpoint"] = toJson(d.startPoint);
	return answer;
}

Json::Value toJson(const TomahawkData& d) {
	Json::Value answer(Json::objectValue);
	answer["target"] = toJson(d.target);
	return answer;
}

Json::Value toJson(const ApacheData& d) {
	Json::Value answer(Json::objectValue);
	answer["firingpattern"] = toJson(d.firingPattern);
	answer["target"] = toJson(d.target);
	return answer;
}

Json::Value toJson(const ExocetData& d) {
	Json::Value answer(Json::objectValue);
	answer["firingpattern"] = toJson(d.firingPattern);
	answer["target"] = toJson(d.target);
	return answer;
}

Json::Value toJson(const RelocateData& d) {
	Json::Value answer(Json::objectValue);
	answer["shipid"] = d.shipId;
	answer["target"] = toJson(d.target);
	return answer;
}

Json::Value toJson(const ScanData& d) {
	Json::Value answer(Json::objectValue);
	answer["target"] = toJson(d.target);
	return answer;
}

Json::Value toJson(const RevealData& d) {
	Json::Value answer(Json::objectValue);
	answer["firingpattern"] = toJson(d.firingPattern);
	return answer;
}

Json::Value toJson(const VehicleAbilityAction& a) {
	Json::Value answer(Json::objectValue);
	answer["type"] = toJson(a.type);

	switch (a.type) {
		case VehicleAbilityType::Torpedo: {
			answer["data"] = toJson(std::get<TorpedoData>(a.data));
			break;
		}
		case VehicleAbilityType::Exocet: {
			answer["data"] = toJson(std::get<ExocetData>(a.data));
			break;
		}
		case VehicleAbilityType::Apache: {
			answer["data"] = toJson(std::get<ApacheData>(a.data));
			break;
		}
		case VehicleAbilityType::Tomahawk: {
			answer["data"] = toJson(std::get<TomahawkData>(a.data));
			break;
		}
		case VehicleAbilityType::relocate: {
			answer["data"] = toJson(std::get<RelocateData>(a.data));
			break;
		}
		case VehicleAbilityType::scan: {
			answer["data"] = toJson(std::get<ScanData>(a.data));
			break;
		}
		case VehicleAbilityType::reveal: {
			answer["data"] = toJson(std::get<RevealData>(a.data));
			break;
		}
	}

	return answer;
}

TorpedoData torpedoDataFromJson(const Json::Value& v) {
	return TorpedoData{
		torpedoFiringPatternFromJson(v["firingpattern"]),
		coordFromJson(v["startpoint"])
	};
}

TomahawkData tomahawkDataFromJson(const Json::Value& v) {
	return TomahawkData{coordFromJson(v["target"])};
}

ApacheData apacheDataFromJson(const Json::Value& v) {
	return ApacheData{
		apacheFiringPatternFromJson(v["firingpattern"]),
		coordFromJson(v["target"])
	};
}

ExocetData exocetDataFromJson(const Json::Value& v) {
	return ExocetData{
		exocetFiringPatternFromJson(v["firingpattern"]),
		coordFromJson(v["target"])
	};
}

RelocateData relocateDataFromJson(const Json::Value& v) {
	return RelocateData{
		v["shipid"].asInt(),
		coordFromJson(v["target"])
	};
}

ScanData scanDataFromJson(const Json::Value& v) {
	return ScanData{coordFromJson(v["target"])};
}

RevealData revealDataFromJson(const Json::Value& v) {
	return RevealData{
		revealFiringPatternFromJson(v["firingpattern"])
	};
}

VehicleAbilityAction vehicleAbilityActionFromJson(const Json::Value& v) {
	VehicleAbilityType type = vehicleAbilityTypeFromJson(v["type"]);
	Json::Value dataJson = v["data"];

	switch (type) {
		case VehicleAbilityType::Torpedo:
			return VehicleAbilityAction{type, torpedoDataFromJson(dataJson)};
		case VehicleAbilityType::Exocet:
			return VehicleAbilityAction{type, exocetDataFromJson(dataJson)};
		case VehicleAbilityType::Apache:
			return VehicleAbilityAction{type, apacheDataFromJson(dataJson)};
		case VehicleAbilityType::Tomahawk:
			return VehicleAbilityAction{type, tomahawkDataFromJson(dataJson)};
		case VehicleAbilityType::relocate:
			return VehicleAbilityAction{type, relocateDataFromJson(dataJson)};
		case VehicleAbilityType::scan:
			return VehicleAbilityAction{type, scanDataFromJson(dataJson)};
		case VehicleAbilityType::reveal:
			return VehicleAbilityAction{type, revealDataFromJson(dataJson)};
	}

	return VehicleAbilityAction{type, TorpedoData{TorpedoData::FiringPattern::vertical, coord::unspecified}};
}

Json::Value toJson(const VehicleAbilityActionData& d) {
	if (std::holds_alternative<TorpedoData>(d)) {
		Json::Value answer(Json::objectValue);
		answer["type"] = "torpedo";
		answer["data"] = toJson(std::get<TorpedoData>(d));
		return answer;
	}
	else if (std::holds_alternative<TomahawkData>(d)) {
		Json::Value answer(Json::objectValue);
		answer["type"] = "tomahawk";
		answer["data"] = toJson(std::get<TomahawkData>(d));
		return answer;
	}
	else if (std::holds_alternative<ApacheData>(d)) {
		Json::Value answer(Json::objectValue);
		answer["type"] = "apache";
		answer["data"] = toJson(std::get<ApacheData>(d));
		return answer;
	}
	else if (std::holds_alternative<ExocetData>(d)) {
		Json::Value answer(Json::objectValue);
		answer["type"] = "exocet";
		answer["data"] = toJson(std::get<ExocetData>(d));
		return answer;
	}
	else if (std::holds_alternative<RelocateData>(d)) {
		Json::Value answer(Json::objectValue);
		answer["type"] = "relocate";
		answer["data"] = toJson(std::get<RelocateData>(d));
		return answer;
	}
	else if (std::holds_alternative<ScanData>(d)) {
		Json::Value answer(Json::objectValue);
		answer["type"] = "scan";
		answer["data"] = toJson(std::get<ScanData>(d));
		return answer;
	}
	else if (std::holds_alternative<RevealData>(d)) {
		Json::Value answer(Json::objectValue);
		answer["type"] = "reveal";
		answer["data"] = toJson(std::get<RevealData>(d));
		return answer;
	}
	return Json::nullValue;
}

Json::Value toJson(const CheckAbilityData& d) {
	Json::Value answer(Json::objectValue);
	answer["vehicleid"] = d.vehicleId;
	answer["abilitydata"] = toJson(d.abilityData);
	return answer;
}

CheckAbilityData checkAbilityDataFromJson(const Json::Value& v) {
	int vehicleId = v["vehicleid"].asInt();
	Json::Value dataJson = v["abilitydata"];
	std::string type = dataJson["type"].asString();
	Json::Value abilityDataContent = dataJson["data"];

	VehicleAbilityActionData abilityData = TorpedoData{TorpedoData::FiringPattern::vertical, coord::unspecified};

	if (type == "torpedo")
		abilityData = torpedoDataFromJson(abilityDataContent);
	else if (type == "exocet")
		abilityData = exocetDataFromJson(abilityDataContent);
	else if (type == "apache")
		abilityData = apacheDataFromJson(abilityDataContent);
	else if (type == "tomahawk")
		abilityData = tomahawkDataFromJson(abilityDataContent);
	else if (type == "relocate")
		abilityData = relocateDataFromJson(abilityDataContent);
	else if (type == "scan")
		abilityData = scanDataFromJson(abilityDataContent);
	else if (type == "reveal")
		abilityData = revealDataFromJson(abilityDataContent);

	return CheckAbilityData{vehicleId, abilityData};
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

Json::Value toJson(const PlacePlaneData& d) {
	Json::Value answer(Json::objectValue);
	answer["position"] = toJson(d.position);
	answer["planeid"] = d.planeId;
	return answer;
}

PlacePlaneData placePlaneDataFromJson(const Json::Value& v) {
	coord c = coordFromJson(v["position"]);
	int i = v["planeid"].asInt();
	return PlacePlaneData{i, c};
}

Json::Value toJson(const SessionActionType& t) {
	Json::Value answer(Json::stringValue);
	switch (t) {
		case SessionActionType::PlaceShip: {
			answer = "placeship";
			break;
		}
		case SessionActionType::PlacePlane: {
			answer = "placeplane";
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
		case SessionActionType::FireAntiAircraft: {
			answer = "fireantiaircraft";
			break;
		}
		case SessionActionType::CheckPlacement: {
			answer = "checkplacement";
			break;
		}
		case SessionActionType::CheckPlanePlacement: {
			answer = "checkplaneplacement";
			break;
		}
		case SessionActionType::CheckAbility: {
			answer = "checkability";
			break;
		}
		case SessionActionType::Rematch: {
			answer = "rematch";
			break;
		}
		case SessionActionType::ActivateAbility: {
			answer = "activateability";
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
	else if (s == "placeplane")
		answer = SessionActionType::PlacePlane;
	else if (s == "ready")
		answer = SessionActionType::Ready;
	else if (s == "fire")
		answer = SessionActionType::Fire;
	else if (s == "fireantiaircraft")
		answer = SessionActionType::FireAntiAircraft;
	else if (s == "checkplacement")
		answer = SessionActionType::CheckPlacement;
	else if (s == "checkplaneplacement")
		answer = SessionActionType::CheckPlanePlacement;
	else if (s == "checkability")
		answer = SessionActionType::CheckAbility;
	else if (s == "rematch")
		answer = SessionActionType::Rematch;
	else if (s == "activateability")
		answer = SessionActionType::ActivateAbility;

	return answer;
}

Json::Value toJson(const SessionActionData& d) {
	if (std::holds_alternative<FireData>(d))
		return toJson(std::get<FireData>(d));
	if (std::holds_alternative<FireAntiAircraftData>(d))
		return toJson(std::get<FireAntiAircraftData>(d));
	if (std::holds_alternative<ReadyData>(d))
		return toJson(std::get<ReadyData>(d));
	if (std::holds_alternative<PlaceShipData>(d))
		return toJson(std::get<PlaceShipData>(d));
	if (std::holds_alternative<PlacePlaneData>(d))
		return toJson(std::get<PlacePlaneData>(d));
	if (std::holds_alternative<RematchData>(d))
		return toJson(std::get<RematchData>(d));
	if (std::holds_alternative<ActivateAbilityData>(d))
		return toJson(std::get<ActivateAbilityData>(d));
	if (std::holds_alternative<CheckAbilityData>(d))
		return toJson(std::get<CheckAbilityData>(d));
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
		case SessionActionResultType::PlacePlaneResult: {
			answer = "placeplaneresult";
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
		case SessionActionResultType::FireAntiAircraftResult: {
			answer = "fireantiaircraft";
			break;
		}
		case SessionActionResultType::TransientOverlayResult: {
			answer = "transientoverlayresult";
			break;
		}
		case SessionActionResultType::RematchResult: {
			answer = "rematchresult";
			break;
		}
		case SessionActionResultType::ActivateAbilityResult: {
			answer = "activateabilityresult";
			break;
		}
	}

	return answer;	
}

Json::Value toJson(const SessionActionResult& r) {
	Json::Value answer(Json::objectValue);

	answer["success"] = r.success;
	answer["actinguser"] = toJson(r.actingUser);
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
	answer["vehicleview"] = toJson(u.vehicleView);
	return answer;
}

Json::Value toJson(const BoardView& b) {
	Json::Value answer(Json::objectValue);
	answer["owngrid"] = toJson(b.ownGrid);
	answer["opponentgrid"] = toJson(b.opponentGrid);
	return answer;
}

Json::Value toJson(const SquareView& s) {
	Json::Value answer;

	answer["coord"] = toJson(s.first);
	answer["state"] = toJson(s.second);

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
		case SquareState::revealedMiss: {
			answer = "revealedmiss";
			break;
		}
		case SquareState::revealedHit: {
			answer = "revealedhit";
			break;
		}
		case SquareState::scannedPositive: {
			answer = "scannedpositive";
			break;
		}
	}
	return answer;
}

Json::Value toJson(const SessionActionResultData& s) {
	Json::Value answer(Json::objectValue);
	if (std::holds_alternative<FireResultData>(s))
		answer = toJson(std::get<FireResultData>(s));
	else if (std::holds_alternative<FireAntiAircraftResultData>(s))
		answer = toJson(std::get<FireAntiAircraftResultData>(s));
	else if (std::holds_alternative<ReadyResultData>(s))
		answer = toJson(std::get<ReadyResultData>(s));
	else if (std::holds_alternative<PlaceShipResultData>(s))
		answer = toJson(std::get<PlaceShipResultData>(s));
	else if (std::holds_alternative<PlacePlaneResultData>(s))
		answer = toJson(std::get<PlacePlaneResultData>(s));
	else if (std::holds_alternative<TransientOverlayData>(s))
		answer = toJson(std::get<TransientOverlayData>(s));
	else if (std::holds_alternative<RematchResultData>(s))
		answer = toJson(std::get<RematchResultData>(s));
	else if (std::holds_alternative<ActivateAbilityResult>(s))
		answer = toJson(std::get<ActivateAbilityResult>(s));
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
		case SessionActionResultError::vehicleNotFound: {
			answer = "vehiclenotfound";
			break;
		}
		case SessionActionResultError::vehicleSunk: {
			answer = "vehiclesunk";
			break;
		}
		case SessionActionResultError::noSuchAbility: {
			answer = "nosuchability";
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

Json::Value toJson(const PlacePlaneResultData& p) {
	return Json::Value(Json::nullValue);
}

Json::Value toJson(const TransientSquareState& s) {
	switch (s) {
		case TransientSquareState::invalidPlacement: return "invalidplacement";
		case TransientSquareState::validPlacement: return "validplacement";
		case TransientSquareState::targetedSquare: return "targetedsquare";
		case TransientSquareState::torpedoUp: return "torpedoup";
		case TransientSquareState::torpedoDown: return "torpedodown";
		case TransientSquareState::torpedoLeft: return "torpedoleft";
		case TransientSquareState::torpedoRight: return "torpedoright";
	}
	return "unknown";
}

Json::Value toJson(const TransientOverlayData& t) {
	Json::Value answer(Json::objectValue);
	Json::Value overlayObj(Json::objectValue);

	for (const auto& [c, states] : t.overlay) {
		Json::Value statesArray(Json::arrayValue);
		for (const auto& state : states)
			statesArray.append(toJson(state));
		overlayObj[std::to_string(c.first.d) + "," + std::to_string(c.first.o) + "," + c.second] = statesArray;
	}

	answer["overlay"] = overlayObj;
	return answer;
}

Json::Value toJson(const RematchResultData& r) {
	return Json::Value(Json::nullValue);
}

Json::Value toJson(const FireAntiAircraftResultData& f) {
	Json::Value answer(Json::objectValue);

	answer["ishit"] = f.isHit;
	answer["isdestroyed"] = f.isDestroyed;
	answer["hitid"] = f.hitId;
	answer["destroyedname"] = f.destroyedName;

	return answer;
}

Json::Value toJson(const ActivateAbilityResultError& e) {
	switch (e) {
		case ActivateAbilityResultError::none: return "none";
		case ActivateAbilityResultError::outOfBounds: return "outofbounds";
		case ActivateAbilityResultError::notYourTurn: return "notyourturn";
		case ActivateAbilityResultError::notYourShip: return "notyourship";
		case ActivateAbilityResultError::shipSunk: return "shipsunk";
		case ActivateAbilityResultError::noSuchAbility: return "nosuchability";
	}
	return "unknown";
}

Json::Value toJson(const TorpedoResultData& d) {
	Json::Value answer(Json::objectValue);
	answer["resulttype"] = "torpedo";
	answer["ishit"] = d.isHit;
	return answer;
}

Json::Value toJson(const ExocetResultData& d) {
	Json::Value answer(Json::objectValue);
	answer["resulttype"] = "exocet";
	answer["ishit"] = d.isHit;
	return answer;
}

Json::Value toJson(const ApacheResultData& d) {
	Json::Value answer(Json::objectValue);
	answer["resulttype"] = "apache";
	answer["ishit"] = d.isHit;
	return answer;
}

Json::Value toJson(const TomahawkResultData& d) {
	Json::Value answer(Json::objectValue);
	answer["resulttype"] = "tomahawk";
	answer["ishit"] = d.isHit;
	return answer;
}

Json::Value toJson(const RelocateResultData& d) {
	Json::Value answer(Json::objectValue);
	answer["resulttype"] = "relocate";
	answer["shipid"] = d.shipId;
	return answer;
}

Json::Value toJson(const ScanResultData& d) {
	Json::Value answer(Json::objectValue);
	answer["resulttype"] = "scan";
	answer["isfound"] = d.isFound;
	return answer;
}

Json::Value toJson(const RevealResultData& d) {
	Json::Value answer(Json::objectValue);
	answer["resulttype"] = "reveal";
	answer["hitsrevealed"] = toJson(d.hitsRevealed);
	return answer;
}

Json::Value toJson(const ActivateAbilityResultData& d) {
	if (std::holds_alternative<TorpedoResultData>(d))
		return toJson(std::get<TorpedoResultData>(d));
	if (std::holds_alternative<ExocetResultData>(d))
		return toJson(std::get<ExocetResultData>(d));
	if (std::holds_alternative<ApacheResultData>(d))
		return toJson(std::get<ApacheResultData>(d));
	if (std::holds_alternative<TomahawkResultData>(d))
		return toJson(std::get<TomahawkResultData>(d));
	if (std::holds_alternative<RelocateResultData>(d))
		return toJson(std::get<RelocateResultData>(d));
	if (std::holds_alternative<ScanResultData>(d))
		return toJson(std::get<ScanResultData>(d));
	if (std::holds_alternative<RevealResultData>(d))
		return toJson(std::get<RevealResultData>(d));
	return Json::nullValue;
}

Json::Value toJson(const ActivateAbilityResult& r) {
	Json::Value answer(Json::objectValue);

	answer["success"] = r.success;
	answer["error"] = toJson(r.error);
	answer["data"] = toJson(r.data);

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
	answer["boardrows"] = s.boardRows;
	answer["boardcols"] = s.boardCols;
	answer["hasantiaircraft"] = s.hasAntiAircraftGun;
	return answer;
}

Json::Value toJson(const UserSnapshot& u) {
	Json::Value answer(Json::objectValue);
	answer["phase"] = toJson(u.phase);
	answer["currentturn"] = toJson(u.currentUser);
	answer["winner"] = toJson(u.winner);
	answer["userview"] = toJson(u.userView);
	answer["youready"] = u.youReady;
	answer["opponentready"] = u.opponentReady;
	answer["hasantiaircraft"] = u.hasAntiAircraftGun;
	return answer;
}

Json::Value toJson(const FleetView& f) {
	Json::Value answer(Json::objectValue);

	Json::Value shipViews(Json::arrayValue);
	for (const ShipView& s : f.ships)
		shipViews.append(toJson(s));
	answer["ships"] = shipViews;

	Json::Value planeViews(Json::arrayValue);
	for (const PlaneView& p : f.planes)
		planeViews.append(toJson(p));
	answer["planes"] = planeViews;

	return answer;
}

Json::Value toJson(const ShipView& s) {
	Json::Value answer(Json::objectValue);
	answer["name"] = toJson(s.name);
	answer["id"] = s.id;
	answer["coords"] = toJson(s.shape);
	answer["issunk"] = s.isSunk;
	if(s.pos)
		answer["position"] = toJson(s.pos.value());
	if(s.rotation)
		answer["rotation"] = s.rotation.value();
	Json::Value abilities(Json::arrayValue);
	for (const VehicleAbility& a : s.abilities)
		abilities.append(toJson(a));
	answer["abilities"] = abilities;

	return answer;
}

Json::Value toJson(const PlaneView& p) {
	Json::Value answer(Json::objectValue);
	answer["name"] = toJson(p.name);
	answer["id"] = p.id;
	answer["isdestroyed"] = p.isDestroyed;
	answer["isoncarrier"] = p.isOnShip;
	if (p.pos)
		answer["position"] = toJson(p.pos.value());
	Json::Value abilities(Json::arrayValue);
	for (const VehicleAbility& a : p.abilities)
		abilities.append(toJson(a));
	answer["abilities"] = abilities;
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
	answer["connectiontoken"] = toJson(r.connectionToken);

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
		gameIdFromJson(v["gameid"]),
		v.isMember("connectiontoken") && v["connectiontoken"].isString() ? v["connectiontoken"].asString() : ""
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

Json::Value toJson(const VehicleView& v) {
	Json::Value answer(Json::objectValue);
	answer["yourfleet"] = toJson(v.yourFleet);
	answer["opponentfleet"] = toJson(v.opponentFleet);
	return answer;
}

} // namespace NavalBattle
