//BattleshipWebSocketManager.cpp
#include "BattleshipWebSocketManager.h"

namespace {
	BattleshipWebSocketManager globalManager;
}

BattleshipWebSocketManager& getBattleshipWebSocketManager() {
	return globalManager;
}

BattleshipWebSocketManager::ConnectionMaps BattleshipWebSocketManager::connectionMaps{};

void BattleshipWebSocketManager::onMessage(const drogon::WebSocketConnectionPtr& conn, std::string&& message) {
	UserId u = userForSocket(conn);
	MessageResult messageResult;
	if (u == "") //socket not bound to user
		messageResult = _messageRouter.onUnauthenticatedMessage(std::move(message));
	else
		messageResult = _messageRouter.onAuthenticatedMessage(u, std::move(message));
	processMessageResultFromConn(messageResult, conn);
}

void BattleshipWebSocketManager::onConnect(const drogon::HttpRequestPtr& req, const drogon::WebSocketConnectionPtr& conn) {
	//nothing to do, wait for first message
}

void BattleshipWebSocketManager::onDisconnect(const drogon::WebSocketConnectionPtr& conn) {
	unbindSocket(conn);
}

bool BattleshipWebSocketManager::bindUserToSocket(const UserId& u, const drogon::WebSocketConnectionPtr& conn) {
	if (socketForUser(u)) //only one connection per user to prevent cheating when UserIds are not authenticated
		return false;
	connectionMaps.socketToUserIdMap[conn] = u;
	connectionMaps.userIdToSocketMap[u] = conn;
	return true;
}

void BattleshipWebSocketManager::unbindSocket(const drogon::WebSocketConnectionPtr& conn) {
	UserId u = userForSocket(conn);
	if (u == "")
		return;
	connectionMaps.userIdToSocketMap.erase(u);
	connectionMaps.socketToUserIdMap.erase(conn);
}

void BattleshipWebSocketManager::sendToUser(const UserId& u, const std::string& s) {
	sendToSocket(socketForUser(u), s);
}

void BattleshipWebSocketManager::sendToSocket(const drogon::WebSocketConnectionPtr& conn, const std::string& s) {
	if(conn)
		conn->send(s);
}

UserId BattleshipWebSocketManager::userForSocket(const drogon::WebSocketConnectionPtr& conn) {
	auto p = connectionMaps.socketToUserIdMap.find(conn);
	return p == connectionMaps.socketToUserIdMap.end() ? "" : p->second;
}

drogon::WebSocketConnectionPtr BattleshipWebSocketManager::socketForUser(const UserId& u) {
	auto p = connectionMaps.userIdToSocketMap.find(u);
	return p == connectionMaps.userIdToSocketMap.end() ? nullptr : p->second;
}

void BattleshipWebSocketManager::processMessageResultFromConn(const MessageResult& m, const drogon::WebSocketConnectionPtr& conn) {
	if (m.senderAction == SenderAction::Bind)
		bindUserToSocket(m.replyingUser, conn);
	if (!m.directReply.empty())
		sendToSocket(conn, m.directReply);
	for (const auto& userAndMessages : m.repliesByUserId)
		for (const auto& message : userAndMessages.second)
			sendToUser(userAndMessages.first, message);
}
