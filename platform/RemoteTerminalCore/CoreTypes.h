#pragma once

#include <string>
#include <string_view>
#include <variant>

// Fundamental identity types used across the RemoteTerminal system
using UserId = std::string;
using GameId = std::string;

// Message addressing types - used to route outbound messages
struct ToSender {};

struct ToUser {
	UserId userId;
};

using OutboundMessageReciever = std::variant<ToSender, ToUser>;

struct InboundMessage {
	std::string_view rawMessage;
	//can add type info later if needed
};
