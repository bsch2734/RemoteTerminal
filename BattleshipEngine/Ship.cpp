#include "Ship.h"


int Ship::nextID = 0;

Ship::Ship(std::vector<coord> coords, int rotation /*= 0*/, coord pos /*= coord::unspecified*/) :
	rotation(rotation),
	coords(coords),
	pos(pos),
	ID(nextID++)
{
};

Ship::Ship(const Ship& other) :
	rotation(other.rotation),
	coords(other.coords),
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

int Ship::getRotation() const {
	return rotation;
}

void Ship::setRotation(int rotation) {
	this->rotation = rotation % 4;
}

coord Ship::getPos() const {
	return pos;
}

void Ship::setPos(coord pos) {
	this->pos = pos;
}

const std::vector<coord>& Ship::getCoords() const {
	return coords;
}

Ship const Ship::carrier{
	{
		coord({0,0}),
		coord({0,1}),
		coord({0,2}),
		coord({0,3}),
		coord({0,4})
	}
};

Ship const Ship::battleship{
	{
		coord({0,0}),
		coord({0,1}),
		coord({0,2}),
		coord({0,3})
	}
};

Ship const Ship::destroyer{
	{
		coord({0,0}),
		coord({0,1}),
		coord({0,2}),
	}
};

Ship const Ship::sub{
	{
		coord({0,0}),
		coord({0,1}),
		coord({0,2})
	}
};

Ship const Ship::pt{
	{
		coord({0,0}),
		coord({0,1})
	}
};