#pragma once

#include "GameEntities.h"
#include "AdvancedGameEntities.h"
#include "AdvancedFleet.h"
#include "Ship.h"
#include "coord.h"
#include <bitset>
#include <unordered_map>
#include "Fleet.h"

namespace Battleship {

    class AdvancedNavalBattleEngine {
    public:
        AdvancedNavalBattleEngine();

        // --- Setup ---

        PlaceShipResult placeShip(Player p, int ID, coord pos, int rotation);

        ValidatePlacementResult validatePlacement(Player p, int ID, coord pos, int rotation) const;

        ReadyUpResult readyUp(Player p);

        bool isPlayerReady(Player p);

        // --- Gameplay ---
        FireResult fire(Player p, coord target);

        ActivateAbilityResult activateAbility(Player p, int shipId, const ShipAbilityAction& activateAbilityAction);

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
        Fleet::hitFleetResult hitCoord(Player p, coord target);
        bool checkCoord(Player p, coord where);

        Phase _phase;
        Player _currentPlayer;

        std::pair<int, int> _boardDimensions;

        struct PlayerData {
                bool isReady = false;
                AdvancedFleet fleet;
                std::set<coord> hits;
                std::set<coord> misses;
                std::set<coord> revealedHits;
                std::set<coord> revealedMisses;
                std::set<std::set<coord>> scansWithHits;
        };

        PlayerData _p1Data;
        PlayerData _p2Data;
        PlayerData _pNoneData;

        bool p1IsReady = false;
        bool p2IsReady = false;

        Fleet const& getBaseFleet();

        enum class FleetStatusBits {
            outOfBounds = 0,
            overlapping = 1,
            unplaced = 2
        };

        std::bitset<8> checkFleetStatus(Fleet f);

        void clearScansWithSquareForPlayer(Player p, coord c);

        PlayerData& getDataForPlayer(Player p);
    };

} // namespace Battleship
