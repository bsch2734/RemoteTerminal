#pragma once

#include "GameEntities.h"
#include "Fleet.h"
#include "Ship.h"
#include "coord.h"
#include <bitset>
#include <unordered_map>

namespace Battleship {

    class AdvancedNavalBattleEngine {
    public:
        AdvancedNavalBattleEngine();

        // --- Setup ---
        const Fleet& getFleetForPlayer(Player p) const;

        PlaceShipResult placeShip(Player p, int ID, coord pos, int rotation);

        ValidatePlacementResult validatePlacement(Player p, int ID, coord pos, int rotation) const;

        ReadyUpResult readyUp(Player p);

        bool isPlayerReady(Player p);

        // --- Gameplay ---
        FireResult fire(Player p, coord target);

        ActivateAbilityResult activateAbility(Player p, int shipId, const ActivateAbilityAction& activateAbilityAction);

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

        // --- Ability Handlers ---
        ActivateAbilityResult handleTorpedoAction(Player p, TorpedoData d);
        ActivateAbilityResult handleExocetAction(Player p, ExocetData d);
        ActivateAbilityResult handleApacheAction(Player p, ApacheData d);
        ActivateAbilityResult handleRelocateAction(Player p, RelocateData d);
        ActivateAbilityResult handleScanAction(Player p, ScanData d);
        ActivateAbilityResult handleRevealAction(Player p, RevealData d);

        // --- Utility ---
        bool hitCoord(Player p, coord target);
        bool checkCoord(Player p, coord where);

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
            unplaced = 2
        };

        std::bitset<8> checkFleetStatus(Fleet f);
    };

} // namespace Battleship
