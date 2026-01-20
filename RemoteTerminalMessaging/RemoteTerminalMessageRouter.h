#pragma once

#include "BattleshipSessionManager.h"
#include <vector>
#include <map>

using OutboundWireMessage = std::string;

struct AddressedWireMessage {
    OutboundMessageReciever address;
    OutboundWireMessage message;
};

class AddressedWireMessageBundle {
public:
    AddressedWireMessageBundle& addMessage(const OutboundMessageReciever& reciever, const OutboundWireMessage& message) {
        messages.emplace_back(AddressedWireMessage{ reciever, message });
        return *this;
    }

    AddressedWireMessageBundle& addMessageBundle(const AddressedWireMessageBundle& bundle) {
        for (const auto& msg : bundle.messages)
            messages.emplace_back(msg);
        return *this;
    }

    auto begin() { return messages.begin(); }
    auto end() { return messages.end(); }

    auto begin() const { return messages.begin(); }
    auto end() const { return messages.end(); }

private:
    std::vector<AddressedWireMessage> messages;
};

struct WireMessageResult {
    SenderAction senderAction = SenderAction::None; //what the session manager requests to happen to the sender
    UserId userToBind; //include the name of the user who sent the message
	AddressedWireMessageBundle addressedMessages; //messages to be sent in the format they travel on the wire
};

class RemoteTerminalMessageRouter {
public:
    WireMessageResult onUnauthenticatedMessage(std::string&& message);

    WireMessageResult onAuthenticatedMessage(const UserId& userID, std::string&& message);

private:
	BattleshipSessionManager _sessionManager;

	AddressedWireMessageBundle routeMessagesToWireFormat(const AddressedMessageBundle& b);

};
