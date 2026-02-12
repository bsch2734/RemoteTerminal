#pragma once
#include "Fleet.h"
#include "GameEntities.h"

class AdvancedFleet : public fleet {
	bool wouldBeHit(const coord& c);
};
