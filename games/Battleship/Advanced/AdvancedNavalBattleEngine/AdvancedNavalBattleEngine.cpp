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
                        return handleTorpedoAction(std::get<TorpedoData>(activateAbilityAction.data));
                        break;
                }
                case ShipAbilityActionType::Exocet: {
                        return handleTorpedoAction(std::get<ExocetData>(activateAbilityAction.data));
                        break;
                }
                case ShipAbilityActionType::Apache: {
                        return handleTorpedoAction(std::get<ApacheData>(activateAbilityAction.data));
                        break;
                }
                case ShipAbilityActionType::relocate: {
                        return handleTorpedoAction(std::get<RelocateData>(activateAbilityAction.data));
                        break;
                }
                case ShipAbilityActionType::scan: {
                        return handleTorpedoAction(std::get<ScanData>(activateAbilityAction.data));
                        break;
                }
                case ShipAbilityActionType::reveal: {
                        return handleTorpedoAction(std::get<RevealData>(activateAbilityAction.data));
                        break;
                }
        }

        return answer;
}

ActivateAbilityResult handleTorpedoAction(Plyaer p, TorpedoData d) {
        ActivateAbilityResult answer;

        coord currentPos = d.startPoint;

        std::function<(void)> incrementPos;


        if (d.FiringPattern == vertical) {
                //downwards
                if (currentPos.d == 0) {
                        incrementPos = [&currentPos]() {currentPos.d++};
                }
                //upwards
                else if (currentPos.d == _boardDimensions.first - 1) {
                        incrementPos = [&currentPos]() {currentPos.d--};
                }
                //invalid
                else {
                        answer.success = false;
                        return answer;
                }
        }
        else {
                //rightwards
                if (currentPos.o == 0) {
                        incrementPos = [&currentPos]() {currentPos.o++};
                }
                //leftwards
                else if (currentPos.o == _boardDimensions.second - 1) {
                        incrementPos = [&currentPos]() {currentPos.o--};

                }
                //invalid
                else {
                        answer.success = false;
                        return answer;
                }
        }

        FireResult r;
        do {
                r = fire(p, currentPos);
                incrementPos();
        } while (r.success && r.isHit == false);

        return answer;
}
