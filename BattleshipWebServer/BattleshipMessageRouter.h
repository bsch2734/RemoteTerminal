#pragma once

#include "BattleshipSessionManager.h"
#include <json/json.h>
#include <vector>
#include <map>

enum class SenderAction {
	None,
	RejectMessage,
	TerminateSession,
	Bind
};

struct MessageResult {
	SenderAction senderAction = SenderAction::None; //what the router requests to happen to the sender
	UserId replyingUser; //include the name of the user who sent the message
	std::string directReply; //replies for ONLY the same connection, not propigated out to any other connections this user has
	std::map<UserId, std::vector<std::string>> repliesByUserId;
};

class BattleshipMessageRouter {
public:
	MessageResult onUnauthenticatedMessage(std::string&& message);

	MessageResult onAuthenticatedMessage(const UserId& userID, std::string&& message);

private:
	static Json::Value parseJson(const std::string& s);

	BattleshipSessionManager _sessionManager;
};
