#pragma once

#include "coord.h"
#include <vector>
#include <map>

enum class Player {
	none = 0,
	one = 1,
	two = 2
};

inline Player oponent(Player p) {
	return (p == Player::one) ? Player::two : Player::one;
}

enum class Phase {
	setup,
	playing,
	finished
};

enum class FireError {
	outOfBounds,
	notYourTurn
};

struct FireResult {
	bool success;
	bool isHit;
	bool isSink;
	int hitId;
	FireError error;
};

enum class PlaceShipError {
	WrongPhase,
	OverlapsAnotherShip,
	OutOfBounds,
	invalidID
};

struct PlaceShipResult {
	bool success;
	PlaceShipError error;
};

enum class ReadyUpError {
	fleetNotPlaced,
	fleetPlacementInvalid
};

struct ReadyUpResult {
	bool succes;
	ReadyUpError error;
};

enum class squareStates {
	miss,
	ship,
	hit
};

struct GridView {
	std::map<coord, squareStates> occupiedSquares;
};

struct BoardView {
	GridView ownGrid;
	GridView oponentGrid;
};