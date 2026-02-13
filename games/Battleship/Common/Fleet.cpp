#include "Fleet.h"
#include <vector>

using namespace Battleship;

Fleet::Fleet(std::vector<Ship> ships) :
	ships(ships)
{};

void Fleet::buildHitmap() {
	hitmap.clear();
	for (Ship& s : ships) {
		for (coord c : s.getCoords()) { //maybe should only add unhit coords?
			coord transformed = c.applyTransform(s.getPos(), s.getRotation());
			hitmap[transformed] = &s;
		}
	}
	hitmapUpToDate = true;
}

std::map<coord, Ship*>& Fleet::getHitmap() {
	if (!hitmapUpToDate)
		buildHitmap();

	return hitmap;
}

std::vector<Ship>& Fleet::getShips() {
	return ships;
}

const std::vector<Ship>& Fleet::getShips() const{
	return ships;
}

Ship* Fleet::getShipWithId(int shipId) {
	Ship* answer = nullptr;
	for (Ship& s : ships)
		if (s.getID() == shipId) {
			answer = &s;
			break;
		}

	return answer;
}

Fleet::Fleet() {}

Fleet::hitFleetResult Fleet::hitFleet(coord c) {
	hitFleetResult answer;

	auto m = getHitmap();
	auto r = m.find(c);
	if (r == m.end()) {
		answer.success = false;
		answer.error = hitFleetError::coordNotInFleet;
		return answer;
	}

	Ship::hitShipResult result = (*r).second->hit(c);
	if (result.success) {
		answer.success = true;
		answer.hitID = (*r).second->getID();
		answer.sunk = result.sunk;
	}
	else {
		if (result.error == Ship::hitShipError::alreadyHit) {
			answer.success = false;
			answer.error = hitFleetError::coordAlreadyHit;
		}
		if (result.error == Ship::hitShipError::notOnShip) {
			answer.success = false;
			answer.error = hitFleetError::coordNotInFleet;
		}
	}
	
	return answer;
}

bool Fleet::isDefeated() const{
	for (const Ship& s : ships)
		if (!s.isSunk())
			return false;
	return true;
}