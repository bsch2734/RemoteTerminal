#pragma once
#include "BattleshipSession.h"
#include "BattleshipMessageRouter.h"
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

    struct ConnectionMaps {
        std::map<drogon::WebSocketConnectionPtr, UserId> socketToUserIdMap;
        std::map<UserId, drogon::WebSocketConnectionPtr> userIdToSocketMap;
    };

    void processMessageResultFromConn(const MessageResult& m, const drogon::WebSocketConnectionPtr& conn);

    bool bindUserToSocket(const UserId& u, const drogon::WebSocketConnectionPtr& conn);

    void unbindSocket(const drogon::WebSocketConnectionPtr& conn);

    void sendToUser(const UserId& u, const std::string& s);

    void sendToSocket(const drogon::WebSocketConnectionPtr& conn, const std::string& s);

    UserId userForSocket(const drogon::WebSocketConnectionPtr& conn);

    drogon::WebSocketConnectionPtr socketForUser(const UserId& u);

    BattleshipMessageRouter _messageRouter;

    static ConnectionMaps connectionMaps;
};

BattleshipWebSocketManager& getBattleshipWebSocketManager();
