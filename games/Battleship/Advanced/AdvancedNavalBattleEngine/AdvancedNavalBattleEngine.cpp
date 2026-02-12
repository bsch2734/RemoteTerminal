#include "AdvancedNavalBattleEngine.h"

ActivateAbilityResult AdvancedNavalBatle::activateAbility(Player p, int shipId, const ActivateAbilityAction& activateAbilityAction a) {
        ActivateAbilityResult answer;

        if (_currentPlayer != p) {
                answer.success = false;
                answer.error = ActivateAbilityError::notYourTurn;
                return answer;
        }

        auto& f = getMutableFleetForPlayer(p);

        Ship* s;
        if (s == nullptr) {
                answer.success = false;
                answer.error = ActivateAbilityError::notYourShip;
                return answer;
        }

        if (s->isSunk) {
                answer.success = false;
                answer.error = ActivateAbilityError::shipSunk;
                return answer;
        }

        if (!s->hasAbility(activateAbility.type)) {
                answer.success = false;
                answer.error = ActicateAbilityError::noSuchAbility;
                return answer;
        }

        switch (activateAbilityAction.type) {
                case ShipAbilityActionType::Torpedo: {
                        answer = handleTorpedoAction(std::get<TorpedoData>(activateAbilityAction.data));
                        break;
                }
                case ShipAbilityActionType::Exocet: {
                        answer = handleExocetAction(std::get<ExocetData>(activateAbilityAction.data));
                        break;
                }
                case ShipAbilityActionType::Apache: {
                        answer = handleApacheAction(std::get<ApacheData>(activateAbilityAction.data));
                        break;
                }
                case ShipAbilityActionType::relocate: {
                        answer = handleRelocateAction(std::get<RelocateData>(activateAbilityAction.data));
                        break;
                }
                case ShipAbilityActionType::scan: {
                        answer = handleScanAction(std::get<ScanData>(activateAbilityAction.data));
                        break;
                }
                case ShipAbilityActionType::reveal: {
                        answer = handleRevealAction(std::get<RevealData>(activateAbilityAction.data));
                        break;
                }
        }

        if(answer.success)
                _currentPlayer = opponent(p);

        return answer;
}

ActivateAbilityResult AdvancedNavalBattleEngine::handleTorpedoAction(Plyaer p, TorpedoData d) {
        ActivateAbilityResult answer;
        TorpedoActionResultData data;

        coord currentPos = d.startPoint;

        std::function<(void)> incrementPos;

        if (d.FiringPattern == vertical)
                //downwards
                if (currentPos.d == 0)
                        incrementPos = [&currentPos]() {currentPos.d++};
                //upwards
                else if (currentPos.d == _boardDimensions.first - 1)
                        incrementPos = [&currentPos]() {currentPos.d--};
                //invalid
                else {
                        answer.success = false;
                        return answer;
                }
        else
                //rightwards
                if (currentPos.o == 0)
                        incrementPos = [&currentPos]() {currentPos.o++};
                //leftwards
                else if (currentPos.o == _boardDimensions.second - 1)
                        incrementPos = [&currentPos]() {currentPos.o--};
                //invalid
                else {
                        answer.success = false;
                        return answer;
                }

        FireResult r;
        do {
                r = fire(p, currentPos);
                incrementPos();
        } while (r.success && r.isHit == false);

        return answer;
}

ActivateAbilityResult AdvancedNavalBattleEngine::handleExocetAction(Player p, ExocetData d) {
        ActivateAbilityResult answer;
        ExocetResultData data;

        if (d.target.d < 1 || d.target.d > _boardDimensions.first - 2 || d.target.o < 1 || d.target.o > _boardDimensions.second - 2) {
                answer.success = false;
                return answer;
        }

        answer.success = true;

        for (int d = target.d - 1; d < target.d + 1; d++)
                for (int o = target.o - 1; o < target.o + 1; o++) {
                        FireResult r = fire(p, { d,o });
                        if (r.isHit)
                                data.isHit = true;
                }
        
        answer.data = data;
        return answer;
}

ActivateAbilityResult AdvancedNavalBattle::handleApacheAction(Player p, ApacheData d) {
        ActivateAbilityResult answer;
        ApacheResultData data;
        if (d.firingPattern == ApacheData::FiringPattern::vertical) {
                if (d.target.d < 1 || d.target.d > _boardDimensions.first - 2 || d.target.o < 0 || d.target.o > _boardDimensions.second - 1) {
                        answer.success = false;
                        answer.error = ActivateAbilityResultError::outOfBounds;
                        return answer;
                }
                bool r = hitCoord(opponent(p), d.target);
                r = r | hitCoord(opponent(p), d.target + {-1, 0});
                r = r | hitCoord(opponent(p), d.target + {1, 0});
                data.isHit = r;
        }
        else {
                if (d.target.o < 1 || d.target.o > _boardDimensions.second - 2 || d.target.d < 0 || d.target.d > _boardDimensions.first - 1) {
                        answer.success = false;
                        answer.error = ActivateAbilityResultError::outOfBounds;
                        return answer;
                }
                bool r = hitCoord(opponent(p), d.target);
                r = r | hitCoord(opponent(p), d.target + {0, -1});
                r = r | hitCoord(opponent(p), d.target + {0, 1});
                data.isHit = r;
        }
        
        answer.data = data;
        return answer;
}

ActivateAbilityResult AdvancedNavalBattleEngine::handleRelocateAction(Player p, RelocateData d) {
        ActivateAbilityResult answer;
        RelocateAbilityData data;
        answer.data = data;

        if (d.target.d < 0 || d.target.d > _boardDimenaions.first - 1 || d.target.o < 0 || d.target.o > _boardDimensions.second - 1) {
                answer.success = false;
                answer.error = outOfBounds;
                return answer;
        }

        //ID has already been validated so we should not have to check for nullptr
        static_cast<AdvancedShip*>(getFleetForPlayer(p).getShipWithId(d.shipId))->setPos(d.target);
        answer.success = true;
        
        return answer;
}

ActivateAbilityResult AdvancedNavalBattleEngine::handleScanAction(Player p, ScanData d) {
        ActivateAbilityResult answer;
        ScanResultData data;

        if (d.target.d < 1 || d.target.d > _boardDimensions.first - 2 || d.target.o < 1 || d.target.o > _boardDimensions.second - 2) {
                answer.success = false;
                return answer;
        }

        answer.success = true;

        for (int d = target.d - 1; d < target.d + 1; d++)
                for (int o = target.o - 1; o < target.o + 1; o++)
                        if (static_cast<AdvancedFleet&>(getMutableFleetForPlayer(p)).wouldBeHit({ d,o })) {
                                //modify some state so the scan shows up?
                                //how are scans represented?
                                data.isFound = true;
                                answer.data = data;
                                return answer;
                        }

        //scan is clear
        //this is the same as guessing/ missing all scanned squares
        for (int d = target.d - 1; d < target.d + 1; d++)
            for (int o = target.o - 1; o < target.o + 1; o++)
				hitCoord(opponent(p), { d,o });

        data.isFound = false;
        answer.data = data;
        return answer;
}

ActivateAbilityResult AdvancedNavalBattleEngine::handleRevealAction(Player p, RevealData d) {

}

FireResult AdvancedNavalBattleEngine::fire(Player p, coord target) {
        FireResult answer;

        if (_currentPlayer != p) {
                answer.success = false;
                answer.error = FireError::notYourTurn;
                return answer;
        }

        if (target.d >= _boardDimensions.first
                || target.o >= _boardDimensions.second
                || target.d < 0
                || target.o < 0) {
                answer.success = false;
                answer.error = FireError::outOfBounds;
                return answer;
        }

        auto& f = getMutableFleetForPlayer(opponent(p));
        auto r = f.hitFleet(target);

        if (r.success) {
                answer.success = true;
                answer.isHit = true;
                getHitsForPlayer(p).insert(target);
                answer.isSink = r.sunk;
                answer.hitId = r.hitID;
        }
        else {
                if (r.error != Fleet::hitFleetError::coordAlreadyHit) //not a true miss if this coord was hit before
                        getMissesForPlayer(p).insert(target);
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

// try to hit the coord target on Player p's board
// should only be passed validated input
// returns if it is a hit
bool AdvancedNavalBattleEngine::hitCoord(Player p, coord target) {
        auto& f = getMutableFleetForPlayer(p);
        auto r = f.hitFleet(target);        
}

bool AdvancedNavalBattleEngine::checkCoord(Player p, coord where){
        auto& f = static_cast<AdvancedFleet&>(getMutableFleetForPlayer(p));
        if(f.wouldBeHit())
}
