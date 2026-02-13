#pragma once
#include "Fleet.h"
#include "GameEntities.h"

namespace Battleship {

	class AdvancedFleet : public Fleet {
	public:
		bool wouldBeHit(const coord& c);
	};
}
