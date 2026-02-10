#pragma once

#include "coord.h"
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
}
