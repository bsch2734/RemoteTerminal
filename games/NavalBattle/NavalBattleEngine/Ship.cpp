#include "Ship.h"
#include <algorithm>
#include <iterator>

using namespace NavalBattle;

Ship::Ship(ShipBlueprint blueprint, VehicleId id, int rotation /*= 0*/, coord pos /*= coord::unspecified*/) :
	rotation(rotation),
	coords(blueprint.coords),
	_name(blueprint.name),
	_abilities(blueprint.abilities),
	_canHoldPlanes(blueprint.canHoldPlanes),
	pos(pos),
	_id(id)
{
};

Ship::Ship(const Ship& other) :
	rotation(other.rotation),
	coords(other.coords),
	hits(other.hits),
	_sunk(other._sunk),
	_name(other._name),
	_abilities(other._abilities),
	_canHoldPlanes(other._canHoldPlanes),
	pos(other.pos),
	_id(other._id)
{
}

bool Ship::hasAbility(const VehicleAbilityType& abilityType) const {
	for (const VehicleAbility& a : _abilities)
		if (a.getType() == abilityType && a.canUse())
			return true;
	return false;
}

bool Ship::isSunk() const {
	return _sunk;
}

int Ship::getId() const {
	return _id;
}

std::string Ship::getName() const {
	return _name;
}

void Ship::setRotation(int rotation) {
	this->rotation = rotation % 4;
}

coord Ship::getPos() const {
	return pos;
}

Ship::hitShipResult Ship::hit(coord where) {
	hitShipResult answer;

	coord transformed = where.applyInverseTransform(pos, rotation);
	
	if (hits.find(transformed) != hits.end()) {
		answer.error = hitShipError::alreadyHit;
		answer.success = false;
		return answer;
	}

	std::set<coord> unhit;
	std::set_difference(
		coords.begin(), coords.end(), 
		hits.begin(), hits.end(), 
		std::inserter(unhit, unhit.end()));
	if (unhit.find(transformed) == unhit.end()) {
		answer.success = false;
		answer.error = hitShipError::notOnShip;
		return answer; //oops, you didn't actually hit the ship
	}

	hits.insert(transformed);
	if (unhit.size() == 1) //there was one unhit, you got a hit, must be sunk now
		_sunk = true;

	answer.success = true;
	answer.sunk = _sunk;

	return answer;
}

bool Ship::isPlaced() const{
	return !pos.isUnspecified();
}

void Ship::setPos(coord pos) {
	this->pos = pos;
}

const std::set<coord>& Ship::getCoords() const {
	return coords;
}

std::set<coord> Ship::getAbsoluteCoords() const {
	std::set<coord> result;
	for (const coord& c : coords)
		result.insert(c.applyTransform(pos, rotation));
	return result;
}

bool NavalBattle::Ship::wouldBeHit(coord where) const {
	coord transformed = where.applyInverseTransform(pos, rotation);
	return coords.find(transformed) != coords.end() && hits.find(transformed) == hits.end();
}

bool Ship::useAbility(VehicleAbilityType abilityType) {
	for (VehicleAbility& a : _abilities)
		if (a.getType() == abilityType)
			if (a.use())
				return true;
	return false;
}

const std::vector<VehicleAbility>& Ship::getAbilities() const {
	return _abilities;
}

bool Ship::canHoldPlanes() const {
	return _canHoldPlanes;
}

ShipBlueprint const Ship::carrier{
	{
		coord({0,0}),
		coord({0,1}),
		coord({0,2}),
		coord({0,3}),
		coord({0,4})
	},
	"Aircraft Carrier"
};

ShipBlueprint const Ship::advancedCarrier{
	{
		coord({0,0}),
		coord({0,1}),
		coord({0,2}),
		coord({0,3}),
		coord({0,4})
	},
	"Aircraft Carrier",
	{VehicleAbility(VehicleAbilityType::Exocet, VehicleAbilityUsagePolicy::limited, 2)  },
	true //can hold planes
};

ShipBlueprint const Ship::battleship{
	{
		coord({0,0}),
		coord({0,1}),
		coord({0,2}),
		coord({0,3})
	},
	"Battleship"
};

ShipBlueprint const Ship::advancedBattleship{
	{
		coord({0,0}),
		coord({0,1}),
		coord({0,2}),
		coord({0,3})
	},
	"Battleship",
	{ VehicleAbility(VehicleAbilityType::Tomahawk, VehicleAbilityUsagePolicy::limited, 1) }
};

ShipBlueprint const Ship::destroyer{
	{
		coord({0,0}),
		coord({0,1}),
		coord({0,2}),
	},
	"Destroyer"
};

ShipBlueprint const Ship::advancedDestroyer{
	{
		coord({0,0}),
		coord({0,1}),
		coord({0,2}),
	},
	"Destroyer",
	{VehicleAbility(VehicleAbilityType::Apache, VehicleAbilityUsagePolicy::limited, 2)  }
};

ShipBlueprint const Ship::sub{
	{
		coord({0,0}),
		coord({0,1}),
		coord({0,2})
	},
	"Submarine"
};

ShipBlueprint const Ship::advancedSub{
	{
		coord({0,0}),
		coord({0,1}),
		coord({0,2})
	},
	"Submarine",
	{
		VehicleAbility(VehicleAbilityType::Torpedo, VehicleAbilityUsagePolicy::limited, 2),
		VehicleAbility(VehicleAbilityType::scan, VehicleAbilityUsagePolicy::unlimited)
	}
};

ShipBlueprint const Ship::pt{
	{
		coord({0,0}),
		coord({0,1})
	},
	"PT Boat"
};
