#pragma once

#include "TicTacToeEngine.h"
#include "CoreTypes.h"
#include "SessionTypes.h"
#include <map>

namespace TicTacToe {

class TicTacToeSession {
public:
    TicTacToeSession(const GameId& id, const UserId& playerOneId, const UserId& playerTwoId);

    std::string getGameId() const;

    bool isFinished() const;

    AddressedMessageBundle handleAction(const UserId& user, const SessionAction& action);

    AddressedMessageBundle getSnapshotMessageBundles();

    AddressedMessageBundle getStartupInfoBundles();

    UserId opponentForUser(const UserId& u);

private:
    // Helpers
    Player playerFor(const UserId& user) const;

    SessionActionResult handleMove(Player p, const SessionAction& a);

private:
    GameId _gameId;
    std::map<UserId, Player> _userToPlayerMap;
    std::map<Player, UserId> _playerToUserMap;

    TicTacToeEngine _engine;
};

} // namespace TicTacToe
