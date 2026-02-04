#pragma once

#include "TicTacToeSession.h"
#include "EndpointTypes.h"
#include <map>

struct MessageResult {
    SenderAction senderAction = SenderAction::None;
    UserId userToBind;
    AddressedMessageBundle addressedMessages;
};

class TicTacToeSessionManager {
public:
    MessageResult handleJoinRequest(const JoinRequest& request);

    MessageResult handleActionRequest(const ActionRequest& request);

    void destroySession(GameId g);

    TicTacToeSession* findSession(GameId g);

private:
    std::map<GameId, TicTacToeSession*> _gameIdToSessionMap;
    std::map<GameId, UserId> _lobbyGames;
};
