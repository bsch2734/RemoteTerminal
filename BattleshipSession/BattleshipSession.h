#pragma once

#include "BattleshipEngine.h"
#include "Action.h"
#include "SessionTypes.h"

class BattleshipSession {
public:
    BattleshipSession(const GameId& id, const UserId& playerOneId, const UserId& playerTwoId);

    std::string getGameId() const;

    bool isFinished() const;

    SessionActionResult handleAction(const UserId& user, const SessionAction& action);

    SessionSnapshot getSnapshot();

    UserSnapshot getSnapshotForUser(const UserId& u);

    StartupInfo getStartupInfoForUser(const UserId& u);

    UserId opponentForUser(const UserId& u);

private:
    // Helpers
    Player playerFor(const UserId& user) const;

    //SessionActionResult makeError(SessionError error) const;
    //SessionActionResult makeResultFromEngine(FireResult fireResult);
    //SessionActionResult makeResultFromEngine(PlaceShipResult placeResult);
    //SessionActionResult makeResultNoOp(); // for Ready, etc.

    //PlayerView buildView(Player p) const;

    SessionActionResult handlePlaceShip(Player p, const SessionAction& a);
    SessionActionResult handleFire(Player p, const SessionAction& a);
    SessionActionResult handleReady(Player p);

private:
    GameId _gameId;
    std::map<UserId, Player> _userToPlayerMap;
    std::map<Player, UserId> _playerToUserMap;

    BattleshipEngine _engine;
};
