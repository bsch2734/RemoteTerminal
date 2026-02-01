#pragma once

#include "CoreTypes.h"
#include <vector>
#include <string>

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

enum class SenderAction {
    None,
    RejectMessage,
    TerminateSession,
    Bind
};

struct WireMessageResult {
    SenderAction senderAction = SenderAction::None;
    UserId userToBind;
    AddressedWireMessageBundle addressedMessages;
};
