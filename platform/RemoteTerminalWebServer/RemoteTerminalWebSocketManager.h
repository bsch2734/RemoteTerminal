#pragma once
#include "RemoteTerminalEndpointRegistry.h"
#include <drogon/WebSocketController.h>
#include <string>
#include <map>

class RemoteTerminalWebSocketManager {
public:
    void onMessage(const drogon::WebSocketConnectionPtr& conn, const InboundMessage& im);

    void onConnect(const drogon::HttpRequestPtr& req, const drogon::WebSocketConnectionPtr& conn);

    void onDisconnect(const drogon::WebSocketConnectionPtr& conn);

    void setEndpointRegistry(RemoteTerminalEndpointRegistry* registry);

private:

    struct ConnectionMaps {
        std::map<drogon::WebSocketConnectionPtr, UserId> socketToUserIdMap;
        std::map<UserId, drogon::WebSocketConnectionPtr> userIdToSocketMap;
    };

    void processMessageResultFromConn(const WireMessageResult& m, const drogon::WebSocketConnectionPtr& conn);

    bool bindUserToSocket(const UserId& u, const drogon::WebSocketConnectionPtr& conn);

    void unbindSocket(const drogon::WebSocketConnectionPtr& conn);

    void sendToUser(const UserId& u, const std::string& s);

    void sendToSocket(const drogon::WebSocketConnectionPtr& conn, const std::string& s);

    UserId userForSocket(const drogon::WebSocketConnectionPtr& conn);

    drogon::WebSocketConnectionPtr socketForUser(const UserId& u);

    RemoteTerminalEndpointRegistry* _endpointRegistry;

    static ConnectionMaps connectionMaps;

	static std::map<drogon::WebSocketConnectionPtr, IRemoteTerminalEndpoint*> _connectionToEndpointMap;
};

RemoteTerminalWebSocketManager& getRemoteTerminalWebSocketManager();
