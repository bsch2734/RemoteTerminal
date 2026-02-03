#pragma once
#include "RemoteTerminalWebSocketManager.h"
#include <drogon/WebSocketController.h>

class RemoteTerminalWebSocketController final : public drogon::WebSocketController<RemoteTerminalWebSocketController> {
public:
    void handleNewMessage(const drogon::WebSocketConnectionPtr& conn, std::string&& message, const drogon::WebSocketMessageType& type) override;

    void handleNewConnection(const drogon::HttpRequestPtr& req, const drogon::WebSocketConnectionPtr& conn) override;

    void handleConnectionClosed(const drogon::WebSocketConnectionPtr& conn) override;

    WS_PATH_LIST_BEGIN
        WS_ADD_PATH_VIA_REGEX("^/ws(/.*)?$");
    WS_PATH_LIST_END
};
