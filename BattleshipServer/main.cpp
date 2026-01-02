#include "BattleshipSession.h"
#include "ServerProtocol.h"
#include <iostream>
#include <unordered_map>

int main() {
    std::string gameId = "g1";
    std::string p1 = "p1";
    std::string p2 = "p2";

    std::unordered_map<std::string, BattleshipSession*> gameIdToSessionMap;

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

        std::string inputGameId = gameIdFromJson(root);
        std::string inputUserId = userIdFromJson(root);
        const Action& inputAction = actionFromJson(root);

        BattleshipSession& activeSession = *gameIdToSessionMap[inputGameId];

        SessionResult res = activeSession.handleAction(inputUserId, inputAction);
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
