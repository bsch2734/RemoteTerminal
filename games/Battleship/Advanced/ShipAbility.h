#pragma once

class ShipAbility {
	virtual ~ShipAbility() = default;
	virtual AbilityResult activate() = 0;

};