#include "AdvancedNavalBattleEngine.h"
#include "AdvancedShip.h"

using namespace Battleship;

ActivateAbilityResult AdvancedNavalBattleEngine::activateAbility(Player p, int shipId, const ShipAbilityAction& shipAbilityAction) {
        ActivateAbilityResult answer;

        if (_currentPlayer != p) {
                answer.success = false;
                answer.error = ActivateAbilityResultError::notYourTurn;
                return answer;
        }

        auto& f = getDataForPlayer(p).fleet;

        AdvancedShip* s;
        if (s == nullptr) {
                answer.success = false;
                answer.error = ActivateAbilityResultError::notYourShip;
                return answer;
        }

        if (s->isSunk()) {
                answer.success = false;
                answer.error = ActivateAbilityResultError::shipSunk;
                return answer;
        }
        /*
        if (!s->hasAbility(shipAbilityAction.type)) {
                answer.success = false;
                answer.error = ActivateAbilityResultError::noSuchAbility;
                return answer;
        }
        */
        switch (shipAbilityAction.type) {
                case ShipAbilityActionType::Torpedo: {
                        answer = handleTorpedoAction(p, std::get<TorpedoData>(shipAbilityAction.data));
                        break;
                }
                case ShipAbilityActionType::Exocet: {
                        answer = handleExocetAction(p, std::get<ExocetData>(shipAbilityAction.data));
                        break;
                }
                case ShipAbilityActionType::Apache: {
                        answer = handleApacheAction(p, std::get<ApacheData>(shipAbilityAction.data));
                        break;
                }
                case ShipAbilityActionType::relocate: {
                        answer = handleRelocateAction(p, std::get<RelocateData>(shipAbilityAction.data));
                        break;
                }
                case ShipAbilityActionType::scan: {
                        answer = handleScanAction(p, std::get<ScanData>(shipAbilityAction.data));
                        break;
                }
                case ShipAbilityActionType::reveal: {
                        answer = handleRevealAction(p, std::get<RevealData>(shipAbilityAction.data));
                        break;
                }
        }

        if(answer.success)
                _currentPlayer = opponent(p);

        return answer;
}

ActivateAbilityResult AdvancedNavalBattleEngine::handleTorpedoAction(Player p, TorpedoData d) {
        ActivateAbilityResult answer;
        TorpedoResultData data;

        coord currentPos = d.startPoint;

        std::function<void()> incrementPos;

        if (d.firingPattern == TorpedoData::FiringPattern::vertical)
            //downwards
            if (currentPos.d == 0)
                incrementPos = [&currentPos]() {currentPos.d++; };
            //upwards
            else if (currentPos.d == boardRows() - 1)
                incrementPos = [&currentPos]() {currentPos.d--; };
            //invalid
            else {
                answer.success = false;
                return answer;
            }
        else
            //rightwards
            if (currentPos.o == 0)
                incrementPos = [&currentPos]() {currentPos.o++; };
            //leftwards
            else if (currentPos.o == boardCols() - 1)
                incrementPos = [&currentPos]() {currentPos.o--; };
            //invalid
            else {
                answer.success = false;
                return answer;
            };

        FireResult r;
        do {
                r = fire(p, currentPos);
                incrementPos();
        } while (r.success && r.isHit == false);

        return answer;
}

ActivateAbilityResult AdvancedNavalBattleEngine::handleExocetAction(Player p, ExocetData data) {
        ActivateAbilityResult answer;
        ExocetResultData resultData;

        if (data.target.d < 1 || data.target.d > boardRows() - 2 || data.target.o < 1 || data.target.o > boardCols() - 2) {
                answer.success = false;
                answer.error = ActivateAbilityResultError::outOfBounds;
                return answer;
        }

        answer.success = true;

        for (int d = data.target.d - 1; d < data.target.d + 1; d++)
                for (int o = data.target.o - 1; o < data.target.o + 1; o++) {
                    FireResult r = fire(p, coord({ d, o }));
                    if (r.isHit)
                        resultData.isHit = true;
                }
        
        answer.data = resultData;
        return answer;
}

ActivateAbilityResult AdvancedNavalBattleEngine::handleApacheAction(Player p, ApacheData d) {
        ActivateAbilityResult answer;
        ApacheResultData data;
        if (d.firingPattern == ApacheData::FiringPattern::vertical) {
                if (d.target.d < 1 || d.target.d > boardRows() - 2 || d.target.o < 0 || d.target.o > boardCols() - 1) {
                        answer.success = false;
                        answer.error = ActivateAbilityResultError::outOfBounds;
                        return answer;
                }
                bool r = hitCoord(opponent(p), d.target).success;
                r = r | hitCoord(opponent(p), d.target + coord({ -1, 0 })).success;
                r = r | hitCoord(opponent(p), d.target + coord({ 1, 0 })).success;
                data.isHit = r;
        }
        else {
                if (d.target.o < 1 || d.target.o > boardCols() - 2 || d.target.d < 0 || d.target.d > boardRows() - 1) {
                        answer.success = false;
                        answer.error = ActivateAbilityResultError::outOfBounds;
                        return answer;
                }
                bool r = hitCoord(opponent(p), d.target).success;
                r = r | hitCoord(opponent(p), d.target + coord({0, -1})).success;
                r = r | hitCoord(opponent(p), d.target + coord({0, 1})).success;
                data.isHit = r;
        }
        
        answer.data = data;
        return answer;
}

ActivateAbilityResult AdvancedNavalBattleEngine::handleRelocateAction(Player p, RelocateData d) {
        ActivateAbilityResult answer;
        RelocateResultData data;
        answer.data = data;

        if (d.target.d < 0 || d.target.d > boardRows() - 1 || d.target.o < 0 || d.target.o > boardCols() - 1) {
                answer.success = false;
                answer.error = ActivateAbilityResultError::outOfBounds;
                return answer;
        }

        //ID has already been validated so we should not have to check for nullptr
        getDataForPlayer(p).fleet.getShipWithId(d.shipId)->setPos(d.target);
        answer.success = true;
        
        return answer;
}

ActivateAbilityResult AdvancedNavalBattleEngine::handleScanAction(Player p, ScanData data) {
        ActivateAbilityResult answer;
        ScanResultData resultData;

        if (data.target.d < 1 || data.target.d > boardRows() - 2 || data.target.o < 1 || data.target.o > boardCols() - 2) {
                answer.success = false;
                return answer;
        }

        answer.success = true;

        //generate all squares in the scan pattern
        std::set<coord> scannedSquares;
        for (int d = data.target.d - 1; d < data.target.d + 1; d++)
                for (int o = data.target.o - 1; o < data.target.o + 1; o++)
                        scannedSquares.insert(coord({ d,o }));

        for (const coord& c : scannedSquares)
                if (getDataForPlayer(p).fleet.wouldBeHit(c)) {
                        getDataForPlayer(p).scansWithHits.insert(scannedSquares);

                        resultData.isFound = true;
                        answer.data = resultData;
                        return answer;
                }

        //scan is clear
        //this is the same as guessing/ missing all scanned squares
        for (const coord& c : scannedSquares)
                hitCoord(opponent(p), c);

        resultData.isFound = false;
        answer.data = resultData;
        return answer;
}

ActivateAbilityResult AdvancedNavalBattleEngine::handleRevealAction(Player p, RevealData d) {
        ActivateAbilityResult answer;
        RevealResultData data;

        if (d.target.d < 1 || d.target.d > boardRows() - 2 || d.target.o < 1 || d.target.o > boardCols() - 2) {
                answer.success = false;
                answer.error = ActivateAbilityResultError::outOfBounds;
                return answer;
        }

        std::set<coord> squaresToReveal;
        if (d.firingPattern == RevealData::FiringPattern::square) {
                squaresToReveal.insert(coord({ d.target.d - 1, d.target.o - 1 }));
                squaresToReveal.insert(coord({ d.target.d - 1, d.target.o + 1 }));
                squaresToReveal.insert(coord({ d.target.d + 1, d.target.o - 1 }));
                squaresToReveal.insert(coord({ d.target.d + 1, d.target.o + 1 }));
        }
        else if (d.firingPattern == RevealData::FiringPattern::diamond) {
                squaresToReveal.insert(coord({ d.target.d - 1, d.target.o }));
                squaresToReveal.insert(coord({ d.target.d, d.target.o - 1 }));
                squaresToReveal.insert(coord({ d.target.d, d.target.o + 1 }));
                squaresToReveal.insert(coord({ d.target.d + 1, d.target.o }));
        }
        else {
                answer.success = false;
                answer.error = ActivateAbilityResultError::noSuchAbility;
                return answer;
        }

        for (const coord& c : squaresToReveal)
                if (checkCoord(opponent(p), c)) { // c would be a hit
                        getDataForPlayer(p).revealedHits.insert(c);
                        data.hitsRevealed.insert(c);
                }
                else
                        getDataForPlayer(p).revealedMisses.insert(c);

        answer.data = data;
        return answer;
}

ReadyUpResult AdvancedNavalBattleEngine::readyUp(Player p) {
        ReadyUpResult answer;
        PlayerData& playerData = getDataForPlayer(p);
        auto status = checkFleetStatus(playerData.fleet);
        if (status.any()) {
                answer.success = false;

                if (status.test((int)FleetStatusBits::unplaced))
                        answer.error = ReadyUpError::fleetNotPlaced;
                else if (status.test((int)FleetStatusBits::overlapping))
                        answer.error = ReadyUpError::fleetPlacementInvalid;
                else if (status.test((int)FleetStatusBits::outOfBounds))
                        answer.error = ReadyUpError::fleetPlacementInvalid;

                return answer;
        }
        
        playerData.isReady = true;

        if (getDataForPlayer(Player::one).isReady && getDataForPlayer(Player::two).isReady) {
                _phase = Phase::playing;
                _currentPlayer = Player::one;
        }

        answer.success = true;
        return answer;
}

bool Battleship::AdvancedNavalBattleEngine::isPlayerReady(Player p) {
        return getDataForPlayer(p).isReady;
}

FireResult AdvancedNavalBattleEngine::fire(Player p, coord target) {
        FireResult answer;

        if (_currentPlayer != p) {
                answer.success = false;
                answer.error = FireError::notYourTurn;
                return answer;
        }

        if (target.d >= boardRows()
                || target.o >= boardCols()
                || target.d < 0
                || target.o < 0) {
                answer.success = false;
                answer.error = FireError::outOfBounds;
                return answer;
        }

        Fleet::hitFleetResult r = hitCoord(opponent(p), target);

        auto& f = getDataForPlayer(opponent(p)).fleet;
        //auto r = f.hitFleet(target);

        if (r.success) {
                answer.success = true;
                answer.isHit = true;
                getDataForPlayer(p).hits.insert(target);
                answer.isSink = r.sunk;
                answer.hitId = r.hitID;
        }
        else {
                if (r.error != Fleet::hitFleetError::coordAlreadyHit) //not a true miss if this coord was hit before
                        getDataForPlayer(p).misses.insert(target);
                answer.success = true;
                answer.isHit = false;
        }

        if (f.isDefeated()) {
                _phase = Phase::finished;
                _currentPlayer = Player::none;
        }
        else
                if (answer.success)
                        _currentPlayer = opponent(p);

        return answer;
}

Phase Battleship::AdvancedNavalBattleEngine::phase() const {
        return _phase;
}

Player Battleship::AdvancedNavalBattleEngine::currentTurn() const {
        return _currentPlayer;
}

int Battleship::AdvancedNavalBattleEngine::boardRows() {
        return _boardDimensions.first;
}

int Battleship::AdvancedNavalBattleEngine::boardCols() {
        return _boardDimensions.second;
}

// try to hit the coord target on Player p's board
// should only be passed validated input
// returns if it is a hit
Fleet::hitFleetResult AdvancedNavalBattleEngine::hitCoord(Player p, coord target) {
        auto& f = getDataForPlayer(p).fleet;
        auto r = f.hitFleet(target);
        if (r.success)
                clearScansWithSquareForPlayer(p, target);
        return r;
}

bool AdvancedNavalBattleEngine::checkCoord(Player p, coord where){
        auto& f = getDataForPlayer(p).fleet;
        return f.wouldBeHit(where);
}

void AdvancedNavalBattleEngine::AdvancedNavalBattleEngine::clearScansWithSquareForPlayer(Player p, coord c) {
        std::erase_if(getDataForPlayer(p).scansWithHits, [c](const std::set<coord>& s) {
                return s.contains(c);
                });
}

AdvancedNavalBattleEngine::PlayerData& AdvancedNavalBattleEngine::AdvancedNavalBattleEngine::getDataForPlayer(Player p) {
        if (p == Player::one)
                return _p1Data;
        else if (p == Player::two)
                return _p2Data;

        return _pNoneData;
}
