#pragma once

#include "EndpointTypes.h"

class IRemoteTerminalEndpoint {
public:
	virtual ~IRemoteTerminalEndpoint() = default;

	virtual WireMessageResult onUnauthenticatedMessage(std::string_view message) = 0;

	virtual WireMessageResult onAuthenticatedMessage(const UserId& userID, std::string_view message) = 0;

	virtual std::string routePath() = 0;
};
