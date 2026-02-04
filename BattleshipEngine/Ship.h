#pragma once

#include "GameEntities.h"
#include "coord.h"
#include <string>
#include <set>

namespace Battleship {

class Ship {
private:
	coord pos;
	std::set<coord> coords;
	std::set<coord> hits;
	int rotation;
	bool _sunk = false;
	int ID;
	std::string _name;

	static int nextID;

public:
	enum class hitShipError {
		notOnShip,
		alreadyHit
	};

	struct hitShipResult {
		bool success;
		bool sunk;
		hitShipError error;
	};

	Ship(const Ship& other);
	Ship(std::set<coord> coords, std::string name = "", int rotation = 0, coord pos = coord::unspecified);
	bool isSunk() const;
	int getID() const;
	std::string getName() const;
	int getRotation() const;
	void setRotation(int rotation);
	coord getPos() const;
	void setPos(coord pos);
	hitShipResult hit(coord where);
	bool isPlaced() const;
	const std::set<coord>& getCoords() const;

	const static Ship pt;
	const static Ship sub;
	const static Ship destroyer;
	const static Ship battleship;
	const static Ship carrier;
};

} // namespace Battleship
