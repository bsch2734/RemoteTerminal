#include "BattleshipSession.h"
#include "BattleshipJsonProtocol.h"
#include <iostream>
#include <unordered_map>

int main() {
    GameId gameId = "g1";
    UserId p1 = "p1";
    UserId p2 = "p2";

    std::unordered_map<GameId, BattleshipSession*> gameIdToSessionMap;

    BattleshipSession session(gameId, p1, p2);

    gameIdToSessionMap[gameId] = &session;

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;

        Json::CharReaderBuilder rb;
        Json::Value root;
        std::string errs;
        std::istringstream iss(line);

        if (!Json::parseFromStream(rb, iss, &root, &errs)) {
            std::cout << errs << std::endl;
            continue;
        }

        GameId inputGameId = gameIdFromJson(root["gameid"]);
        UserId inputUserId = userIdFromJson(root["userid"]);
        const SessionAction& inputAction = sessionActionFromJson(root["sessionaction"]);

        BattleshipSession& activeSession = *gameIdToSessionMap[inputGameId];

        SessionActionResult res = activeSession.handleAction(inputUserId, inputAction);
        SessionSnapshot snapshot = activeSession.getSnapshot();

        Json::Value out = toJson(ServerUpdate(res, snapshot));

        // write one JSON line out:
        Json::StreamWriterBuilder wb;
        wb["indentation"] = ""; // single-line
        std::cout << Json::writeString(wb, out) << "\n";
        std::cout.flush();
    }

    return 0;
}
