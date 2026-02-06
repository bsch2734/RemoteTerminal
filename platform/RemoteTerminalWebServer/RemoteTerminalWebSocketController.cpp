#include "RemoteTerminalWebSocketController.h"
#include <drogon/drogon.h>

void RemoteTerminalWebSocketController::handleNewConnection(const drogon::HttpRequestPtr& httpRequest, const drogon::WebSocketConnectionPtr& conn) {
    LOG_INFO << "WS connected: " << conn->peerAddr().toIpPort();
    getRemoteTerminalWebSocketManager().onConnect(httpRequest, conn);
}

void RemoteTerminalWebSocketController::handleConnectionClosed(const drogon::WebSocketConnectionPtr& conn) {
    LOG_INFO << "WS closed: " << conn->peerAddr().toIpPort();
    getRemoteTerminalWebSocketManager().onDisconnect(conn);
}

void RemoteTerminalWebSocketController::handleNewMessage(const drogon::WebSocketConnectionPtr& conn, std::string&& message, const drogon::WebSocketMessageType& type) {
    //actual closing logic is handled in handleConnectionClosed
	//no need to process close messages here
	//also ignore ping/pong messages
    if (   type == drogon::WebSocketMessageType::Close
        || type == drogon::WebSocketMessageType::Ping
        || type == drogon::WebSocketMessageType::Pong)
        return;

    getRemoteTerminalWebSocketManager().onMessage(conn, InboundMessage(std::move(message)));
}
