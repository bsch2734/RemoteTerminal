#include "RemoteTerminalWebSocketManager.h"

namespace {
	RemoteTerminalWebSocketManager globalManager;
}

RemoteTerminalWebSocketManager& getRemoteTerminalWebSocketManager() {
	return globalManager;
}

RemoteTerminalWebSocketManager::ConnectionMaps RemoteTerminalWebSocketManager::connectionMaps{};
std::map<drogon::WebSocketConnectionPtr, IRemoteTerminalEndpoint*> RemoteTerminalWebSocketManager::_connectionToEndpointMap;

void RemoteTerminalWebSocketManager::onMessage(const drogon::WebSocketConnectionPtr& conn, const InboundMessage& im) {
	UserId u = userForSocket(conn);
	WireMessageResult messageResult;
	if (u == "") //socket not bound to user
		messageResult = _connectionToEndpointMap[conn]->onUnauthenticatedMessage(std::move(im.rawMessage));
	else
		messageResult = _connectionToEndpointMap[conn]->onAuthenticatedMessage(u, std::move(im.rawMessage));
	processMessageResultFromConn(messageResult, conn);
}

void RemoteTerminalWebSocketManager::onConnect(const drogon::HttpRequestPtr& req, const drogon::WebSocketConnectionPtr& conn) {
	auto* endpoint = _endpointRegistry->endpointForRoute(req->path());

	if (!endpoint) {
		conn->shutdown();
		return;
	}

	_connectionToEndpointMap[conn] = endpoint;
}

void RemoteTerminalWebSocketManager::onDisconnect(const drogon::WebSocketConnectionPtr& conn) {
	unbindSocket(conn);
	const auto& endpoint = _connectionToEndpointMap.find(conn);
	if (endpoint != _connectionToEndpointMap.end())
		_connectionToEndpointMap.erase(conn);
}

void RemoteTerminalWebSocketManager::setEndpointRegistry(RemoteTerminalEndpointRegistry* registry) {
	_endpointRegistry = registry;
}

bool RemoteTerminalWebSocketManager::bindUserToSocket(const UserId& u, const drogon::WebSocketConnectionPtr& conn) {
	if (socketForUser(u)) //only one connection per user to prevent cheating when UserIds are not authenticated
		return false;
	connectionMaps.socketToUserIdMap[conn] = u;
	connectionMaps.userIdToSocketMap[u] = conn;
	return true;
}

void RemoteTerminalWebSocketManager::unbindSocket(const drogon::WebSocketConnectionPtr& conn) {
	UserId u = userForSocket(conn);
	if (u == "")
		return;
	connectionMaps.userIdToSocketMap.erase(u);
	connectionMaps.socketToUserIdMap.erase(conn);
}

void RemoteTerminalWebSocketManager::sendToUser(const UserId& u, const std::string& s) {
	sendToSocket(socketForUser(u), s);
}

void RemoteTerminalWebSocketManager::sendToSocket(const drogon::WebSocketConnectionPtr& conn, const std::string& s) {
	if(conn)
		conn->send(s);
}

UserId RemoteTerminalWebSocketManager::userForSocket(const drogon::WebSocketConnectionPtr& conn) {
	auto p = connectionMaps.socketToUserIdMap.find(conn);
	return p == connectionMaps.socketToUserIdMap.end() ? "" : p->second;
}

drogon::WebSocketConnectionPtr RemoteTerminalWebSocketManager::socketForUser(const UserId& u) {
	auto p = connectionMaps.userIdToSocketMap.find(u);
	return p == connectionMaps.userIdToSocketMap.end() ? nullptr : p->second;
}

void RemoteTerminalWebSocketManager::processMessageResultFromConn(const WireMessageResult& m, const drogon::WebSocketConnectionPtr& conn) {
	if (m.senderAction == SenderAction::Bind)
		bindUserToSocket(m.userToBind, conn);

	for (const AddressedWireMessage& addressedMessage : m.addressedMessages)
		if (std::holds_alternative<ToSender>(addressedMessage.address))
			sendToSocket(conn, addressedMessage.message);
		else if (std::holds_alternative<ToUser>(addressedMessage.address))
			sendToUser(std::get<ToUser>(addressedMessage.address).userId, addressedMessage.message);
}
