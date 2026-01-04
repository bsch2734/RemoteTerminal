#include "BattleshipWebSocketManager.h"
#include "BattleshipJsonProtocol.h"

namespace {
	BattleshipWebSocketManager globalManager;
}

BattleshipWebSocketManager& getBattleshipWebSocketManager() {
	return globalManager;
}

BattleshipWebSocketManager::ConnectionMaps BattleshipWebSocketManager::connectionMaps{};

BattleshipSessionManager BattleshipWebSocketManager::_sessionManager;

void BattleshipWebSocketManager::onMessage(const drogon::WebSocketConnectionPtr& conn, std::string&& message) {
	auto r = connectionMaps.socketToUserIdMap.find(conn);
	if (r == connectionMaps.socketToUserIdMap.end()) //socket not bound to user, this should be join message
		onJoinMessage(conn, std::move(message));
	else
		onActionMessage(conn, std::move(message));
}

void BattleshipWebSocketManager::onConnect(const drogon::HttpRequestPtr& req, const drogon::WebSocketConnectionPtr& conn) {
	//nothing to do, wait for first message
}

void BattleshipWebSocketManager::onDisconnect(const drogon::WebSocketConnectionPtr& conn) {
	connectionMaps.userIdToSocketMap.erase(connectionMaps.socketToUserIdMap[conn]);
	connectionMaps.socketToUserIdMap.erase(conn);
}

void BattleshipWebSocketManager::onJoinMessage(const drogon::WebSocketConnectionPtr& conn, std::string&& message) {
    Json::Value root = parseJson(message);
    UserId u = root["userid"].asString();
    GameId g = root["gameid"].asString();
    _sessionManager.addUserToGame(u, g);
    connectionMaps.socketToUserIdMap[conn] = u;
    connectionMaps.userIdToSocketMap[u] = conn;
    //no reply for now, client assumes it worked
}

void BattleshipWebSocketManager::onActionMessage(const drogon::WebSocketConnectionPtr& conn, std::string&& message) {
    Json::Value root = parseJson(message);

    GameId inputGameId = gameIdFromJson(root["gameid"]);
    UserId inputUserId = connectionMaps.socketToUserIdMap[conn];
    const SessionAction& inputAction = sessionActionFromJson(root["sessionaction"]);

    BattleshipSession* activeSession = _sessionManager.findSession(inputGameId);

    SessionActionResult handleActionResult = activeSession->handleAction(inputUserId, inputAction);
    UserSnapshot userSnapshot = activeSession->getSnapshotForUser(inputUserId);
    UserId opponentUser = activeSession->opponentForUser(inputUserId);
    UserSnapshot opponentSnapshot= activeSession->getSnapshotForUser(opponentUser);

    Json::Value userOut = toJson(UserUpdate(handleActionResult, userSnapshot));
    Json::Value opponentOut = toJson(UserUpdate(handleActionResult, opponentSnapshot));

    auto& opponentConn = connectionMaps.userIdToSocketMap[opponentUser];

    // write one JSON line out:
    Json::StreamWriterBuilder wb;
    wb["indentation"] = ""; // single-line
    
    conn->send(Json::writeString(wb, userOut));
    opponentConn->send(Json::writeString(wb, opponentOut));
}

Json::Value BattleshipWebSocketManager::parseJson(const std::string& s) {
    Json::CharReaderBuilder rb;
    Json::Value root;
    std::string errs;
    std::istringstream iss(s);

    if (!Json::parseFromStream(rb, iss, &root, &errs))
        return Json::nullValue;

    return root;
}
