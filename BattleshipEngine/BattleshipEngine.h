#pragma once

#include "GameEntities.h"
#include "Fleet.h"
#include "Ship.h"
#include "coord.h"
#include <bitset>

#include <iostream>
class BattleshipEngine {
    public:
        BattleshipEngine();

        // --- Setup ---
        const Fleet& getFleetForPlayer(Player p) const;

        PlaceShipResult placeShip(Player p, int ID, coord pos, int rotation);

        bool readyUp(Player p);

        // --- Gameplay ---
        FireResult fire(Player p, coord target);

        // --- Queries ---
        Phase phase() const;
        Player getWinner() const;
        Player currentTurn() const;

        //BoardView ownBoard(Player p) const;
        //BoardView opponentBoard(Player p) const;

private:
    Fleet& getMutableFleetForPlayer(Player p);

    Phase _phase;
    Player _currentPlayer;
    Fleet _pOneFleet;
    Fleet _pTwoFleet;
    std::pair<int, int> _boardDimensions;

    static Fleet baseFleet;

    enum class FleetStatusBits {
        outOfBounds = 0,
        overlapping = 1,
        unplaced    = 2
    };

    std::bitset<8> checkFleetStatus(Fleet f);
};
