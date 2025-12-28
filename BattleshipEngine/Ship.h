#pragma once

#include "GameEntities.h"
#include "coord.h"


class Ship {
private:
	coord pos;
	std::vector<coord> coords;
	int rotation;
	bool _sunk = false;
	int ID;

	static int nextID;

public:
	Ship(const Ship& other);
	Ship(std::vector<coord> coords, int rotation = 0, coord pos = coord::unspecified);
	bool isSunk() const;
	int getID() const;
	int getRotation() const;
	void setRotation(int rotation);
	coord getPos() const;
	void setPos(coord pos);
	const std::vector<coord>& getCoords() const;

	const static Ship pt;
	const static Ship sub;
	const static Ship destroyer;
	const static Ship battleship;
	const static Ship carrier;
};
