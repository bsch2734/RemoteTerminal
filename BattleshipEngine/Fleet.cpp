#include "fleet.h"
#include <vector>

Fleet::Fleet(std::vector<Ship> ships) :
	ships(ships)
{};

std::vector<Ship>& Fleet::getShips() {
	return ships;
}

const std::vector<Ship>& Fleet::getShips() const{
	return ships;
}

Fleet::Fleet() {};

bool Fleet::isDefeated() const{
	for (Ship s : ships)
		if (!s.isSunk())
			return false;
	return true;
}