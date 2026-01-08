#pragma once

#include "BattleshipEngine.h"
#include "Action.h"
#include "SessionTypes.h"

class BattleshipSession {
public:
    BattleshipSession(GameId id, UserId playerOneId, UserId playerTwoId);

    std::string id() const;

    bool isFinished() const;

    SessionActionResult handleAction(UserId user, const SessionAction& action);

    SessionSnapshot getSnapshot();

    UserSnapshot getSnapshotForUser(UserId u);

    StartupInfo getStartupInfoForUser(UserId u);

    UserId opponentForUser(UserId& u);

private:
    // Helpers
    Player playerFor(UserId user) const;

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
    std::map<UserId, Player> _userToPlayerMap;
    std::map<Player, UserId> _playerToUserMap;

    BattleshipEngine _engine;
};
