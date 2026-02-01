#include "RemoteTerminalEndpointRegistry.h"

void RemoteTerminalEndpointRegistry::registerEndpoint(std::string route, IRemoteTerminalEndpoint* endpoint) {
	_endpoints[route] = endpoint;
}

IRemoteTerminalEndpoint* RemoteTerminalEndpointRegistry::endpointForRoute(const std::string& route) {
	return _endpoints[route];
}
