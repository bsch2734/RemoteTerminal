#pragma once

#include "IRemoteTerminalEndpoint.h"
#include <vector>
#include <map>


class RemoteTerminalEndpointRegistry {
public:
	void registerEndpoint(std::string route, IRemoteTerminalEndpoint* endpoint);

	IRemoteTerminalEndpoint* endpointForRoute(const std::string& route);

private:
	std::map<std::string, IRemoteTerminalEndpoint*> _endpoints;
};
