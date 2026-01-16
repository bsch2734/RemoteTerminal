#include "BattleshipMessageRouter.h"
#include "BattleshipJsonProtocol.h"


WireMessageResult BattleshipMessageRouter::onUnauthenticatedMessage(std::string&& message) {
    WireMessageResult answer;

    JoinRequest inputJoinRequest = joinRequestFromJson(parseJson(message));

	//if an unauthenticated user, assume their message is a join request and send it to session manager
    auto a = _sessionManager.handleJoinRequest(inputJoinRequest);
    answer.addressedMessages = routeMessagesToWireFormat(a.addressedMessages);
    answer.senderAction = a.senderAction;
    answer.userToBind = a.userToBind;

    return answer;
}

WireMessageResult BattleshipMessageRouter::onAuthenticatedMessage(const UserId& userId, std::string&& message) {
    WireMessageResult answer;

	ActionRequest inputActionRequest = actionRequestFromJson(parseJson(message));

    auto a = _sessionManager.handleActionRequest(inputActionRequest);
    
    answer.addressedMessages = routeMessagesToWireFormat(a.addressedMessages);
	answer.senderAction = a.senderAction;
	answer.userToBind = a.userToBind;

    return answer;
}

AddressedWireMessageBundle BattleshipMessageRouter::routeMessagesToWireFormat(const AddressedMessageBundle& b) {
	AddressedWireMessageBundle wireBundle;

    for (const auto& addressedMessage : b) {
        OutboundWireMessage wireMessage = outboundWireMessageFromJson(toJson(addressedMessage.message));
        wireBundle.addMessage(addressedMessage.address, wireMessage);
	}

	return wireBundle;
}
