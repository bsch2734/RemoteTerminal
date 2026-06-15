#include "Fleet.h"
#include <vector>

using namespace NavalBattle;

Fleet::Fleet(std::vector<Ship> ships) :
	ships(ships)
{};

void Fleet::buildHitmap() const{
	hitmap.clear();
	for (const Ship& s : ships)
		for (coord c : s.getAbsoluteCoords()) 
			hitmap[c] = const_cast<Ship*>(&s);

	hitmapUpToDate = true;
}

std::map<coord, Ship*>& Fleet::getHitmap() const{
	if (!hitmapUpToDate)
		buildHitmap();

	return hitmap;
}

Ship* Fleet::getShipById(VehicleId id) {
	for (Ship& s : ships)
		if (s.getId() == id)
			return &s;
	return nullptr;
}

Plane* Fleet::getPlaneById(VehicleId id) {
	for (Plane& p : planes)
		if (p.getId() == id)
			return &p;
	return nullptr;
}

void Fleet::addShip(const Ship& ship) {
	ships.push_back(ship);
	hitmapUpToDate = false;
}

void Fleet::placeShip(VehicleId id, coord pos, int rotation) {
	Ship* s = getShipById(id);
	if (s) {
		s->setPos(pos);
		s->setRotation(rotation);
		hitmapUpToDate = false;
		return;
	}
}

void Fleet::addPlane(const Plane& plane) {
	planes.push_back(plane);
}

void Fleet::placePlane(VehicleId id, coord pos) {
	Plane* p = getPlaneById(id);
	if (p)
		p->setPos(pos);
}

void Fleet::placeVehicle(VehicleId id, coord pos, int rotation /*=0*/) {
	Ship* s = getShipById(id);
	if (s) {
		s->setPos(pos);
		s->setRotation(rotation);
		hitmapUpToDate = false;
		return;
	}
	Plane* p = getPlaneById(id);
	if (p)
		p->setPos(pos);
}

void NavalBattle::Fleet::vehicleIsOnShip(VehicleId id, bool state) {
	Plane* p = getPlaneById(id);
	if (p)
		p->isOnShip(state);
}

bool Fleet::useShipAbility(VehicleId id, VehicleAbilityType abilityType) {
	Ship* s = getShipById(id);
	if (s == nullptr)
		return false;
	return s->useAbility(abilityType);
}

Fleet::AbilityAvailabilityError Fleet::abilityAvailable(VehicleId id, VehicleAbilityType ability) const {
	const Ship* s = getShipById(id);
	if (s != nullptr) {
		if (s->isSunk())
			return AbilityAvailabilityError::vehicleDestroyed;
		if (!s->hasAbility(ability))
			return AbilityAvailabilityError::vehicleHasNoSuchAbility;
		return AbilityAvailabilityError::none;
	}
	const Plane* p = getPlaneById(id);
	if (p != nullptr) {
		if (p->isDestroyed())
			return AbilityAvailabilityError::vehicleDestroyed;
		if (!p->hasAbility(ability))
			return AbilityAvailabilityError::vehicleHasNoSuchAbility;
		
		return AbilityAvailabilityError::none;
	}
	return AbilityAvailabilityError::invalidID;
}

const Ship* Fleet::getShipById(VehicleId id) const {
	return const_cast<Fleet*>(this)->getShipById(id);
}

const Plane* Fleet::getPlaneById(VehicleId id) const {
	return const_cast<Fleet*>(this)->getPlaneById(id);
}

const std::vector<Ship>& Fleet::getShips() const{
	return ships;
}

const std::vector<Plane>& Fleet::getPlanes() const {
	return planes;
}

Fleet::Fleet() {}

Fleet::hitFleetShipsResult Fleet::hitFleetShips(coord c) {
	hitFleetShipsResult answer;

	auto m = getHitmap();
	auto r = m.find(c);
	if (r == m.end()) {
		answer.success = false;
		answer.error = hitFleetShipsError::coordNotInFleet;
		return answer;
	}

	Ship::hitShipResult result = (*r).second->hit(c);
	if (result.success) {
		answer.success = true;
		answer.hitID = (*r).second->getId();
		answer.sunk = result.sunk;
		hitFleetPlanesOnShip(c);
	}
	else {
		if (result.error == Ship::hitShipError::alreadyHit) {
			answer.success = false;
			answer.error = hitFleetShipsError::coordAlreadyHit;
		}
		if (result.error == Ship::hitShipError::notOnShip) {
			answer.success = false;
			answer.error = hitFleetShipsError::coordNotInFleet;
		}
	}

	return answer;
}

Fleet::hitFleetPlanesResult Fleet::hitFleetPlanesInAir(coord c) {
	hitFleetPlanesResult answer;

	answer.destroyed = false;
	answer.error = hitFleetPlanesError::noPlaneAtCoord;
	answer.success = false;
	answer.hitID = -1;

	for (Plane& plane : planes) {
		if (!plane.isPlaced())
			continue;

		Plane::HitPlaneResult result = plane.hitInAir(c);
		if (result.success) {
			answer.success = true;
			answer.hitID = plane.getId();
			answer.destroyed = result.destroyed;
		}
		// do not break in case there are overlapping planes, we want to hit all of them
		// this can happen if a second plane is where a plane was destroyed
	}

	return answer;
}

Fleet::hitFleetPlanesResult Fleet::hitFleetPlanesOnShip(coord c) {
	hitFleetPlanesResult answer;

	answer.destroyed = false;
	answer.error = hitFleetPlanesError::noPlaneAtCoord;
	answer.success = false;
	answer.hitID = -1;

	for (Plane& plane : planes) {
		if (!plane.isPlaced())
			continue;

		Plane::HitPlaneResult result = plane.hitOnShip(c);
		if (result.success) {
			answer.success = true;
			answer.hitID = plane.getId();
			answer.destroyed = result.destroyed;
		}
		// do not break in case there are overlapping planes, we want to hit all of them
		// this can happen if a second plane is where a plane was destroyed
	}

	return answer;
}

bool Fleet::wouldBeHit(const coord& c) {
	auto m = getHitmap();
	auto r = m.find(c);
	if (r == m.end() || !r->second->wouldBeHit(c))
		return false;
	return true;
}

bool Fleet::isDefeated() const{
	for (const Ship& s : ships)
		if (!s.isSunk())
			return false;
	return true;
}
