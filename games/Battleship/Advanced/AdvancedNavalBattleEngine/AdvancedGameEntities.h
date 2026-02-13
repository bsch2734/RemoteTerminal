#pragma once

#include "coord.h"
#include <variant>
#include <vector>
#include <map>
#include <set>

namespace Battleship {
	enum class ShipAbilityActionType{
		//firing
		Torpedo,
		Exocet,
		Apache,

		//movement
		relocate,

		//scanning
		scan,  //check if there is anything in the area, yes or no
		reveal //report the exact states of all squares
	} ;

	struct TorpedoData {
		enum class FiringPattern {
			vertical,
			horizontal
		};
		FiringPattern firingPattern;
		coord startPoint;
	};

	struct ExocetData {
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
		coord target;
	};

	using ShipAbilityActionData = std::variant<
		TorpedoData,
		ExocetData,
		ApacheData,
		RelocateData,
		ScanData,
		RevealData>;

	struct ShipAbilityAction {
		ShipAbilityActionType type;
		ShipAbilityActionData data;
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
		bool isHit = false;
	};

	struct ExocetResultData {
		bool isHit = false;
	};

	struct ApacheResultData {
		bool isHit = false;
	};

	struct RelocateResultData {}; //no data

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
		RelocateResultData,
		ScanResultData,
		RevealResultData
	>;

	struct ActivateAbilityResult {
		bool success = false;
		ActivateAbilityResultError error = ActivateAbilityResultError::none;
		ActivateAbilityResultData data;
	};
}
