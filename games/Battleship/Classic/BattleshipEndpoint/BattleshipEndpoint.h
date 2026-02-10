#pragma once

#include "IRemoteTerminalEndpoint.h"
#include "BattleshipSessionManager.h"
#include "SessionTypes.h"

class BattleshipEndpoint : public IRemoteTerminalEndpoint {
public:
	virtual WireMessageResult onUnauthenticatedMessage(std::string_view message) override;

	virtual WireMessageResult onAuthenticatedMessage(const UserId& userID, std::string_view message) override;

	virtual std::string routePath() override;

private:
	Battleship::BattleshipSessionManager _sessionManager;

	AddressedWireMessageBundle routeMessagesToWireFormat(const Battleship::AddressedMessageBundle& b);
};
