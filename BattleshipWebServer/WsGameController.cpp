#include "WsGameController.h"
#include <drogon/drogon.h>

void WsGameController::handleNewConnection(const drogon::HttpRequestPtr&,
    const drogon::WebSocketConnectionPtr& conn)
{
    LOG_INFO << "WS connected: " << conn->peerAddr().toIpPort();
    conn->send("connected"); // simple handshake message
}

void WsGameController::handleConnectionClosed(const drogon::WebSocketConnectionPtr& conn)
{
    LOG_INFO << "WS closed: " << conn->peerAddr().toIpPort();
}

void WsGameController::handleNewMessage(const drogon::WebSocketConnectionPtr& conn,
    std::string&& message,
    const drogon::WebSocketMessageType& type)
{
    if (type == drogon::WebSocketMessageType::Text) {
        conn->send(std::move(message));
        return;
    }

    conn->send("text only");
}
