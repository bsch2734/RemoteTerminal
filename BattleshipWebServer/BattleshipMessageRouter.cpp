#include "BattleshipMessageRouter.h"
#include "BattleshipJsonProtocol.h"


MessageResult BattleshipMessageRouter::onUnauthenticatedMessage(std::string&& message) {
    MessageResult answer;
    //if an unauthenticated user, assume their message is a join request
    Json::Value root = parseJson(message);
    //we are assuming root is a Json::Object
    Json::Value userIdJson = root.find("userid");
    Json::Value gameIdJson = root.find("gameid");
    if (userIdJson.type() != Json::stringValue || gameIdJson.type() != Json::stringValue) {
        answer.replyingUser = "";
        answer.directReply = "{\"error\":\"missinginfo\"}";
        answer.senderAction = SenderAction::RejectMessage;
        return answer;
    }
    UserId u = userIdJson.asString();
    GameId g = gameIdJson.asString();
    answer.replyingUser = u;
    //need to check if this user is already connected and reject if so
    AddUserToGameResult r = _sessionManager.addUserToGame(u, g);
    if (r.success) {
        answer.senderAction = SenderAction::Bind;
        //both users here, send them start message
        if (r.readyToStart) {
            BattleshipSession* activeSession = _sessionManager.findSession(g);
            UserId opponentUser = activeSession->opponentForUser(u);

            StartupInfo userStartupInfo = activeSession->getStartupInfoForUser(u);
            StartupInfo opponentStartupInfo = activeSession->getStartupInfoForUser(opponentUser);

            //userStartupInfo needs to be the value of a key setupinfo

            Json::StreamWriterBuilder wb;
            wb["indentation"] = ""; // single-line

            //tell user game is startin
            Json::Value out(Json::objectValue);
            std::vector<std::string> replies;
            out["setupinfo"] = toJson(userStartupInfo);
            replies.push_back(Json::writeString(wb, out));
            answer.repliesByUserId[u] = replies;

            //tell opponent game is starting
            out.clear();
            replies.clear();
            out["setupinfo"] = toJson(opponentStartupInfo);
            replies.push_back(Json::writeString(wb, out));
            answer.repliesByUserId[opponentUser] = replies;
        }
        //first user only, send waiting message
        else {
            answer.directReply = "{\"waiting\":\"true\"}";
        }
    }
    return answer;
}

MessageResult BattleshipMessageRouter::onAuthenticatedMessage(const UserId& userId, std::string&& message) {
    MessageResult answer;
    answer.replyingUser = userId;

    Json::Value root = parseJson(message);
    //we are assuming root is a Json::Object
    Json::Value gameIdJson = root.find("gameid");
    Json::Value sessionActionJson = root.find("sessionaction");
    if (gameIdJson.type() != Json::stringValue || sessionActionJson.type() != Json::objectValue) {
        answer.directReply = "{\"error\":\"missinginfo\"}";
        answer.senderAction = SenderAction::RejectMessage;
        return answer;
    }
    
    GameId inputGameId = gameIdJson.asString();
    const SessionAction& inputAction = sessionActionFromJson(sessionActionJson);

    BattleshipSession* activeSession = _sessionManager.findSession(inputGameId);

    SessionActionResult handleActionResult = activeSession->handleAction(userId, inputAction);
    UserSnapshot userSnapshot = activeSession->getSnapshotForUser(userId);

    Json::Value userOut(Json::objectValue);
    userOut["snapshot"] = toJson(userSnapshot);
    userOut["actionresult"] = toJson(handleActionResult);

    // write one JSON line out:
    Json::StreamWriterBuilder wb;
    wb["indentation"] = ""; // single-line    
    
    //conn->send(Json::writeString(wb, userOut));
    std::vector<std::string> replies;
    replies.push_back(Json::writeString(wb, userOut));
    answer.repliesByUserId[userId] = replies;

    //no need to send opponent updates if action failed (nothing has changed)
    if (handleActionResult.success) {
        UserId opponentUser = activeSession->opponentForUser(userId);
        UserSnapshot opponentSnapshot = activeSession->getSnapshotForUser(opponentUser);
        Json::Value opponentOut(Json::objectValue);
        opponentOut["snapshot"] = toJson(opponentSnapshot);
        opponentOut["actionresult"] = toJson(handleActionResult);
        replies.clear();
        replies.push_back(Json::writeString(wb, opponentOut));
        answer.repliesByUserId[opponentUser] = replies;
    }
    return answer;
}

Json::Value BattleshipMessageRouter::parseJson(const std::string& s) {
    Json::CharReaderBuilder rb;
    Json::Value root;
    std::string errs;
    std::istringstream iss(s);

    if (!Json::parseFromStream(rb, iss, &root, &errs))
        return Json::nullValue;

    return root;
}
