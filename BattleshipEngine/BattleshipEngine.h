#pragma once

#include "GameEntities.h"
#include "Fleet.h"
#include "Ship.h"
#include "coord.h"
#include <bitset>
#include <unordered_map>

class BattleshipEngine {
    public:
        BattleshipEngine();

        // --- Setup ---
        const Fleet& getFleetForPlayer(Player p) const;

        PlaceShipResult placeShip(Player p, int ID, coord pos, int rotation);

        ValidatePlacementResult validatePlacement(Player p, int ID, coord pos, int rotation) const;

        ReadyUpResult readyUp(Player p);

        bool isPlayerReady(Player p);

        // --- Gameplay ---
        FireResult fire(Player p, coord target);

        // --- Queries ---
        Phase phase() const;
        Player getWinner() const;
        Player currentTurn() const;
        std::string nameForId(int id) const;

        int boardRows();
        int boardCols();

        const std::set<coord>& getHitsForPlayer(Player p) const;
        const std::set<coord>& getMissesForPlayer(Player p) const;

        BoardView boardViewForPlayer(Player p) const;

private:
    Fleet& getMutableFleetForPlayer(Player p);

    std::set<coord>& getHitsForPlayer(Player p);
    std::set<coord>& getMissesForPlayer(Player p);

    GridView ownGrid(Player p) const;
    GridView opponentGrid(Player p) const;

    Phase _phase;
    Player _currentPlayer;
    Fleet _pOneFleet;
    Fleet _pTwoFleet;
    std::set<coord> _p1Hits;
    std::set<coord> _p1Misses;
    std::set<coord> _p2Hits;
    std::set<coord> _p2Misses;
    std::pair<int, int> _boardDimensions;

    bool p1IsReady = false;
    bool p2IsReady = false;

    Fleet const& getBaseFleet();

    enum class FleetStatusBits {
        outOfBounds = 0,
        overlapping = 1,
        unplaced    = 2
    };

    std::bitset<8> checkFleetStatus(Fleet f);
};
