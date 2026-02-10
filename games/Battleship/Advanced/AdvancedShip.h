#pragma once

#include "Ship.h"
#include "ShipAbility.h"

namespace Battleship {
	class AdvancedShip : public Ship {
		std::vector<ShipAbility> abilities;
	};
} //namespace Battleship
