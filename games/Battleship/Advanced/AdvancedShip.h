#pragma once

#include "Ship.h"
//#include "ShipAbility.h"

namespace Battleship {
	class AdvancedShip : public Ship {
		AdvancedShip();

		//std::vector<ShipAbility> abilities;
		//bool hasAbility(ShipAbility ability);

		bool isPlane;
	};
} //namespace Battleship
