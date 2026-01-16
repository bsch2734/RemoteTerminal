#pragma once
#include "BattleshipSession.h"
#include <json/json.h>
#include <map>

enum class SenderAction {
	None,
	RejectMessage,
	TerminateSession,
	Bind
};

struct MessageResult {
	SenderAction senderAction = SenderAction::None; //what the session manager requests to happen to the sender
	UserId userToBind; //include the name of the user who sent the message
	AddressedMessageBundle addressedMessages; //messages to be sent in the format they travel on the wire
};

class BattleshipSessionManager {
public:
	MessageResult handleJoinRequest(const JoinRequest& request);

	MessageResult handleActionRequest(const ActionRequest& request);

	void destroySession(GameId g);

	BattleshipSession* findSession(GameId g);

private:

	std::map<GameId, BattleshipSession*> _gameIdToSessionMap;
	std::map<GameId, UserId> _lobbyGames;
};
