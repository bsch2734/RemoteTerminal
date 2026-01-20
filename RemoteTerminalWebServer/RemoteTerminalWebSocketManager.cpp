#include "RemoteTerminalWebSocketManager.h"

namespace {
	RemoteTerminalWebSocketManager globalManager;
}

RemoteTerminalWebSocketManager& getRemoteTerminalWebSocketManager() {
	return globalManager;
}

RemoteTerminalWebSocketManager::ConnectionMaps RemoteTerminalWebSocketManager::connectionMaps{};

void RemoteTerminalWebSocketManager::onMessage(const drogon::WebSocketConnectionPtr& conn, std::string&& message) {
	UserId u = userForSocket(conn);
	WireMessageResult messageResult;
	if (u == "") //socket not bound to user
		messageResult = _messageRouter.onUnauthenticatedMessage(std::move(message));
	else
		messageResult = _messageRouter.onAuthenticatedMessage(u, std::move(message));
	processMessageResultFromConn(messageResult, conn);
}

void RemoteTerminalWebSocketManager::onConnect(const drogon::HttpRequestPtr& req, const drogon::WebSocketConnectionPtr& conn) {
	//nothing to do, wait for first message
}

void RemoteTerminalWebSocketManager::onDisconnect(const drogon::WebSocketConnectionPtr& conn) {
	unbindSocket(conn);
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
