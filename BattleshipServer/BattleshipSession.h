#pragma once

#include "BattleshipEngine.h"
#include "Action.h"
#include "SessionTypes.h"

class BattleshipSession {
public:
    BattleshipSession(std::string id, std::string playerOneId, std::string playerTwoId);

    std::string id() const;

    bool isFinished() const;

    SessionActionResult handleAction(std::string user, const SessionAction& action);

    SessionSnapshot getSnapshot();

private:
    // Helpers
    Player playerFor(std::string user) const;

    //SessionActionResult makeError(SessionError error) const;
    //SessionActionResult makeResultFromEngine(FireResult fireResult);
    //SessionActionResult makeResultFromEngine(PlaceShipResult placeResult);
    //SessionActionResult makeResultNoOp(); // for Ready, etc.

    //PlayerView buildView(Player p) const;

    SessionActionResult handlePlaceShip(Player p, const SessionAction& a);
    SessionActionResult handleFire(Player p, const SessionAction& a);
    SessionActionResult handleReady(Player p);

private:
    std::string _sessionId;
    std::map<std::string, Player> _userToPlyaerMap;
    std::map<Player, std::string> _playerToUserMap;

    BattleshipEngine _engine;
};
