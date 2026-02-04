#pragma once

#include "IRemoteTerminalEndpoint.h"
#include "TicTacToeSessionManager.h"
#include "SessionTypes.h"

class TicTacToeEndpoint : public IRemoteTerminalEndpoint {
public:
    virtual WireMessageResult onUnauthenticatedMessage(std::string_view message) override;

    virtual WireMessageResult onAuthenticatedMessage(const UserId& userID, std::string_view message) override;

    virtual std::string routePath() override;

private:
    TicTacToeSessionManager _sessionManager;

    AddressedWireMessageBundle routeMessagesToWireFormat(const AddressedMessageBundle& b);
};
