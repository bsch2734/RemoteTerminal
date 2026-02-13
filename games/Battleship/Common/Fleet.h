#pragma once

#include "Ship.h"
#include<map>

//#include "GameEntities.h"

namespace Battleship {

class Fleet {
protected:
	std::vector<Ship> ships;
	void buildHitmap();
	std::map<coord, Ship*>& getHitmap();
	bool hitmapUpToDate = false;
	std::map<coord, Ship*> hitmap;

public:
	enum class hitFleetError {
		coordNotInFleet,
		coordAlreadyHit
	};

	struct hitFleetResult {
		bool success;
		int hitID;
		bool sunk;
		hitFleetError error;
	};

	std::vector<Ship>& getShips();
	const std::vector<Ship>& getShips() const;

	Ship* getShipWithId(int Id);
	
	Fleet(std::vector<Ship> ships);
	Fleet();

	hitFleetResult hitFleet(coord c);

	bool isDefeated() const;


};

} // namespace Battleship