#include "WsGameController.h"
#include <drogon/drogon.h>

void WsGameController::handleNewConnection(const drogon::HttpRequestPtr& httpRequest, const drogon::WebSocketConnectionPtr& conn) {
    LOG_INFO << "WS connected: " << conn->peerAddr().toIpPort();
    getRemoteTerminalWebSocketManager().onConnect(httpRequest, conn);
}

void WsGameController::handleConnectionClosed(const drogon::WebSocketConnectionPtr& conn) {
    LOG_INFO << "WS closed: " << conn->peerAddr().toIpPort();
    getRemoteTerminalWebSocketManager().onDisconnect(conn);
}

void WsGameController::handleNewMessage(const drogon::WebSocketConnectionPtr& conn, std::string&& message, const drogon::WebSocketMessageType& type) {
    //only pass through text messages
    if (type == drogon::WebSocketMessageType::Text) {
        getRemoteTerminalWebSocketManager().onMessage(conn, std::move(message));
        return;
    }

    conn->send("text only");
}
