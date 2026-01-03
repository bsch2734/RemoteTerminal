#pragma once
#include <drogon/WebSocketController.h>

class WsGameController final : public drogon::WebSocketController<WsGameController> {
public:
    void handleNewMessage(const drogon::WebSocketConnectionPtr& conn,
        std::string&& message,
        const drogon::WebSocketMessageType& type) override;

    void handleNewConnection(const drogon::HttpRequestPtr& req,
        const drogon::WebSocketConnectionPtr& conn) override;

    void handleConnectionClosed(const drogon::WebSocketConnectionPtr& conn) override;

    WS_PATH_LIST_BEGIN
        WS_PATH_ADD("/ws");   // ws://host:8080/ws
    WS_PATH_LIST_END
};