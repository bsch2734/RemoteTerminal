#pragma once

#include "Ship.h"

//#include "GameEntities.h"

class Fleet {
private:
	std::vector<Ship> ships;

public:
	std::vector<Ship>& getShips();
	const std::vector<Ship>& getShips() const;
	Fleet(std::vector<Ship> ships);
	Fleet();

	bool isDefeated() const;
};