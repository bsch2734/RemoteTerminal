#pragma once
#include "BattleshipSession.h"
#include "BattleshipSessionManager.h"
#include <drogon/WebSocketController.h>
#include <string>
#include <map>

class BattleshipWebSocketManager {
public:
    void onMessage(const drogon::WebSocketConnectionPtr& conn, std::string&& message);

    void onConnect(const drogon::HttpRequestPtr& req, const drogon::WebSocketConnectionPtr& conn);

    void onDisconnect(const drogon::WebSocketConnectionPtr& conn);

private:
    void onJoinMessage(const drogon::WebSocketConnectionPtr& conn, std::string&& message);

    void onActionMessage(const drogon::WebSocketConnectionPtr& conn, std::string&& message);

    struct ConnectionMaps {
        std::map<drogon::WebSocketConnectionPtr, UserId> socketToUserIdMap;
        std::map<UserId, drogon::WebSocketConnectionPtr> userIdToSocketMap;
    };

    static Json::Value parseJson(const std::string& s);

    static BattleshipSessionManager _sessionManager;

    static ConnectionMaps connectionMaps;
};

BattleshipWebSocketManager& getBattleshipWebSocketManager();