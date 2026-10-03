#pragma once

#include "coord.h"
#include "VehicleAbility.h"
#include <string>
#include <variant>
#include <vector>
#include <optional>
#include <map>
#include <set>

namespace NavalBattle {

enum class Player {
	none = 0,
	one = 1,
	two = 2
};

inline Player opponent(Player p) {
	return (p == Player::one) ? Player::two : Player::one;
}

enum class Phase {
	setup,
	playing,
	finished
};

enum class FireError {
	outOfBounds,
	notYourTurn,
	none
};

struct FireResult {
	bool success;
	bool isHit;
	bool isSink;
	int hitId;
	FireError error;
};

enum class PlaceShipError {
	WrongPhase,
	OverlapsAnotherShip,
	OutOfBounds,
	invalidID
};

struct PlaceShipResult {
	bool success;
	PlaceShipError error;
};

enum class PlacePlaneError {
	WrongPhase,
	OverlapsAnotherPlane,
	NotOnCarrier,
	invalidID
};

struct PlacePlaneResult {
	bool success;
	PlacePlaneError error;
};

struct ValidateShipPlacementResult {
	bool valid;
	PlaceShipError error;
	std::set<coord> coords;
};

struct ValidatePlanePlacementResult {
	bool valid;
	PlacePlaneError error;
	coord position = coord::unspecified;
};

enum class ReadyUpError {
	fleetNotPlaced,
	fleetPlacementInvalid
};

struct ReadyUpResult {
	bool success;
	ReadyUpError error;
};

enum class SquareState {
	miss,
	hit,
	revealedMiss,
	revealedHit,
	scannedPositive
};

using GridView = std::map<coord, SquareState>;
using SquareView = GridView::value_type;

struct BoardView {
	GridView ownGrid;
	GridView opponentGrid;
};

struct ShipView {
	int id;
	std::string name;
	std::set<coord> shape;
	std::optional<coord> pos;
	std::optional<int> rotation;
	bool isSunk;
	std::vector<VehicleAbility> abilities;
};

struct PlaneView {
	int id;
	std::string name;
	std::optional<coord> pos;
	bool isOnShip;
	bool isDestroyed;
	std::vector<VehicleAbility> abilities;
};

struct FleetView {
	std::vector<ShipView> ships;
	std::vector<PlaneView> planes;
};

struct VehicleView {
	FleetView yourFleet;
	FleetView opponentFleet;
};

struct TorpedoData {
	enum class FiringPattern {
		vertical,
		horizontal
	};
	FiringPattern firingPattern;
	coord startPoint;
};

struct TomahawkData {
	coord target;
};

struct ApacheData {
	enum class FiringPattern {
		vertical,
		horizontal
	};
	FiringPattern firingPattern;
	coord target;
};

struct ExocetData {
	enum class FiringPattern {
		plus,
		x
	};
	FiringPattern firingPattern;
	coord target;
};

struct RelocateData {
	int shipId;
	coord target;
};

struct ScanData {
	coord target;
};

struct RevealData {
	enum class FiringPattern {
		square,
		diamond
	};
	FiringPattern firingPattern;
};

struct BulkFireData {
	std::set<coord> targets;
};

using VehicleAbilityActionData = std::variant<
	TorpedoData,
	TomahawkData,
	ApacheData,
	ExocetData,
	RelocateData,
	ScanData,
	RevealData
>;

struct VehicleAbilityAction {
	VehicleAbilityType type;
	VehicleAbilityActionData data;
};

enum class ActivateAbilityResultError {
	none,
	outOfBounds,
	notYourTurn,
	notYourShip,
	shipSunk,
	noSuchAbility
};

struct TorpedoResultData {
	bool isHit;
};

struct ExocetResultData {
	bool isHit;
};

struct ApacheResultData {
	bool isHit;
};

struct TomahawkResultData {
	bool isHit;
};

struct RelocateResultData { int shipId = -1; };

struct ScanResultData {
	bool isFound = false;
};

struct RevealResultData {
	std::set<coord> hitsRevealed;
};

using ActivateAbilityResultData = std::variant<
	TorpedoResultData,
	ExocetResultData,
	ApacheResultData,
	TomahawkResultData,
	RelocateResultData,
	ScanResultData,
	RevealResultData
>;

struct ActivateAbilityResult {
	bool success = false;
	ActivateAbilityResultError error = ActivateAbilityResultError::none;
	ActivateAbilityResultData data;
};


//plans: these are internal, execution ready actions
//however, they may or may not be valid and so should
//be used with a ValidateAbilityResult which 
//encapsulates whether the plan is valid
struct TorpedoPlan {
	enum class TorpedoDirection {
		left,
		right,
		up,
		down
	};
	coord startPoint = coord::unspecified;
	TorpedoDirection direction;
};

struct ExocetPlan {
	std::set<coord> targets;
};

struct ApachePlan {
	std::set<coord> targets;
};

struct TomahawkPlan {
	std::set<coord> targets;
};

struct RelocatePlan {
	int shipId;
	coord target = coord::unspecified;
	bool willBeOnShip = false;
};

struct ScanPlan {
	std::set<coord> targets;
};

struct RevealPlan {
	std::set<coord> targets;
};

using AbilityPlan = std::variant<
	TorpedoPlan,
	ExocetPlan,
	ApachePlan,
	TomahawkPlan,
	RelocatePlan,
	ScanPlan,
	RevealPlan
>;

struct ValidateAbilityResult {
	ActivateAbilityResultError error = ActivateAbilityResultError::none;
	AbilityPlan plan;
};

using VehicleId = int;

struct AbilityContext {
	VehicleId vehicleId = -1;
};

struct ShipBlueprint {
	std::set<coord> coords;
	std::string name;
	std::vector<VehicleAbility> abilities;
	bool canHoldPlanes = false;
};

struct PlaneBlueprint {
	std::string name;
	std::vector<VehicleAbility> abilities;
};

struct FleetBlueprint {
	std::vector<ShipBlueprint> ships;
	std::vector<PlaneBlueprint> planes;
};

enum class GameMode {
	classic,
	advanced
};

} // namespace NavalBattle
