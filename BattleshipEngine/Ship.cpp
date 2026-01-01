#include "Ship.h"
#include <algorithm>
#include <iterator>


int Ship::nextID = 0;

Ship::Ship(std::set<coord> coords, std::string name /*=""*/, int rotation /*= 0*/, coord pos /*= coord::unspecified*/) :
	rotation(rotation),
	coords(coords),
	_name(name),
	pos(pos),
	ID(nextID++)
{
};

Ship::Ship(const Ship& other) :
	rotation(other.rotation),
	coords(other.coords),
	_name(other._name),
	pos(other.pos),
	ID(nextID++)
{
}

bool Ship::isSunk() const {
	return _sunk;
}

int Ship::getID() const {
	return ID;
}

std::string Ship::getName() const {
	return _name;
}

int Ship::getRotation() const {
	return rotation;
}

void Ship::setRotation(int rotation) {
	this->rotation = rotation % 4;
}

coord Ship::getPos() const {
	return pos;
}

Ship::hitShipResult Ship::hit(coord where) {
	hitShipResult answer;

	if (hits.find(where) != hits.end()) {
		answer.error = hitShipError::alreadyHit;
		answer.success = false;
		return answer;
	}

	std::set<coord> unhit;
	std::set_difference(
		coords.begin(), coords.end(), 
		hits.begin(), hits.end(), 
		std::inserter(unhit, unhit.end()));
	coord transformed = where.applyInverseTransform(pos, rotation);
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

void Ship::setPos(coord pos) {
	this->pos = pos;
}

const std::set<coord>& Ship::getCoords() const {
	return coords;
}

Ship const Ship::carrier{
	{
		coord({0,0}),
		coord({0,1}),
		coord({0,2}),
		coord({0,3}),
		coord({0,4})
	},
	"Aircraft Carrier"
};

Ship const Ship::battleship{
	{
		coord({0,0}),
		coord({0,1}),
		coord({0,2}),
		coord({0,3})
	},
	"Battleship"
};

Ship const Ship::destroyer{
	{
		coord({0,0}),
		coord({0,1}),
		coord({0,2}),
	},
	"Destroyer"
};

Ship const Ship::sub{
	{
		coord({0,0}),
		coord({0,1}),
		coord({0,2})
	},
	"Submarine"
};

Ship const Ship::pt{
	{
		coord({0,0}),
		coord({0,1})
	},
	"PT Boat"
};