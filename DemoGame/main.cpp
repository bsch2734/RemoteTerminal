#include <iostream>
#include <stdlib.h>
#include "BattleshipEngine.h"

int main() {
	BattleshipEngine e;

	for (const Ship& s : e.getFleetForPlayer(Player::one).getShips()) {
		PlaceShipResult r;
		do {
			int rotation = rand() % 4;
			coord pos = { { rand() % 10, rand() % 10 } };
			r = e.placeShip(Player::one, s.getID(), pos, rotation);
		} while (!r.success);
	}

	e.readyUp(Player::one);

	for (const Ship& s : e.getFleetForPlayer(Player::two).getShips()) {
		PlaceShipResult r;
		do {
			int rotation = rand() % 4;
			coord pos = { { rand() % 10, rand() % 10 } };
			r = e.placeShip(Player::two, s.getID(), pos, rotation);
		} while (!r.success);
	}
	e.readyUp(Player::two);

	FireResult r;

	r = e.fire(Player::one, { { 3,1 } });
	r = e.fire(Player::one, { { 3,1 } });
	r = e.fire(Player::two, { { 3,1 } });
	r = e.fire(Player::one, { { 3,1 } });
	r = e.fire(Player::two, { { 3,1 } });
	r = e.fire(Player::one, { { 3,0 } });
	r = e.fire(Player::two, { { 3,0 } });
}
