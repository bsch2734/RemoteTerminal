#pragma once

#include "BattleshipEngine.h"
#include "Action.h"
#include "SessionTypes.h"

namespace Battleship {

class BattleshipSession {
public:
    BattleshipSession(const GameId& id, const UserId& playerOneId, const UserId& playerTwoId);

    std::string getGameId() const;

    bool isFinished() const;

    AddressedMessageBundle handleAction(const UserId& user, const SessionAction& action);

    AddressedMessageBundle getSnapshotMessageBundles();

    AddressedMessageBundle getStartupInfoMessageBundles();

	AddressedMessageBundle getSnapshotMessageBundleForUser(const UserId& u);

    UserId opponentForUser(const UserId& u);

private:
    // Helpers
    Player playerFor(const UserId& user) const;

    SessionActionResult handlePlaceShip(Player p, const SessionAction& a);
    SessionActionResult handleFire(Player p, const SessionAction& a);
    SessionActionResult handleReady(Player p);
    SessionActionResult handleCheckPlacement(Player p, const SessionAction& a);
    SessionActionResult handleRematch(Player p);
    AddressedMessageBundle processRematchRequest(const UserId& user, Player p, const SessionActionResult& result);

private:
    GameId _gameId;
    std::map<UserId, Player> _userToPlayerMap;
    std::map<Player, UserId> _playerToUserMap;

    BattleshipEngine _engine;
    
    // Rematch tracking
    bool _playerOneWantsRematch = false;
    bool _playerTwoWantsRematch = false;
};

} // namespace Battleship
