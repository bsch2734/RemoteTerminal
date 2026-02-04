#include "BattleshipEndpoint.h"
#include "BattleshipJsonProtocol.h"

using namespace Battleship;

WireMessageResult BattleshipEndpoint::onUnauthenticatedMessage(std::string_view message) {
	WireMessageResult answer;

	JoinRequest inputJoinRequest = joinRequestFromJson(parseJson(message));

	//if an unauthenticated user, assume their message is a join request and send it to session manager
	auto a = _sessionManager.handleJoinRequest(inputJoinRequest);
	answer.addressedMessages = routeMessagesToWireFormat(a.addressedMessages);
	answer.senderAction = a.senderAction;
	answer.userToBind = a.userToBind;

	return answer;
}

WireMessageResult BattleshipEndpoint::onAuthenticatedMessage(const UserId& userID, std::string_view message) {
	WireMessageResult answer;

	if (message.empty())
		return answer;

	ActionRequest inputActionRequest = actionRequestFromJson(parseJson(message));

	auto a = _sessionManager.handleActionRequest(inputActionRequest);

	answer.addressedMessages = routeMessagesToWireFormat(a.addressedMessages);
	answer.senderAction = a.senderAction;
	answer.userToBind = a.userToBind;

	return answer;
}

AddressedWireMessageBundle BattleshipEndpoint::routeMessagesToWireFormat(const AddressedMessageBundle& b) {
	AddressedWireMessageBundle wireBundle;

	for (const auto& addressedMessage : b) {
		OutboundWireMessage wireMessage = outboundWireMessageFromJson(toJson(addressedMessage.message));
		wireBundle.addMessage(addressedMessage.address, wireMessage);
	}

	return wireBundle;
}

std::string BattleshipEndpoint::routePath() {
	return "com.titohq.navalbattle";
}
