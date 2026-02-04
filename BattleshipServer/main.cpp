#include "BattleshipEndpoint.h"
#include "BattleshipJsonProtocol.h"
#include <iostream>
#include <unordered_map>

using namespace Battleship;

int main() {
    BattleshipEndpoint messageRouter;
    
    // Track which userIds have been "authenticated" (bound) in this CLI session
    // Maps a "connection id" (line number or similar) to userId
    // For CLI, we use a simple approach: once a user joins, they're bound
    std::unordered_map<UserId, bool> boundUsers;
    
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;

        Json::Value root = parseJson(line);
        if (root.isNull()) {
            std::cerr << "Invalid JSON" << std::endl;
            continue;
        }

        WireMessageResult messageResult;

        // Check if this is a join request (has userid and gameid but no sessionaction)
        // or an action request (has userid, gameid, and sessionaction)
        bool hasSessionAction = root.isMember("sessionaction") && !root["sessionaction"].isNull();
        UserId userId = root.isMember("userid") ? userIdFromJson(root["userid"]) : "";

        // Determine if user is already authenticated/bound
        bool isAuthenticated = boundUsers.find(userId) != boundUsers.end() && boundUsers[userId];

        if (!hasSessionAction) {
            // Treat as join request (unauthenticated message)
            messageResult = messageRouter.onUnauthenticatedMessage(std::move(line));
        } else if (isAuthenticated) {
            // Treat as action request (authenticated message)
            messageResult = messageRouter.onAuthenticatedMessage(userId, std::move(line));
        } else {
            // User trying to perform action without joining first
            std::cerr << "User not authenticated. Send a join request first." << std::endl;
            continue;
        }

        // Process sender action (bind user if requested)
        if (messageResult.senderAction == SenderAction::Bind) {
            boundUsers[messageResult.userToBind] = true;
        }

        // Output all addressed messages
        for (const AddressedWireMessage& addressedMessage : messageResult.addressedMessages) {
            // For CLI, we output all messages with a prefix indicating the recipient
            Json::Value output(Json::objectValue);
            
            if (std::holds_alternative<ToSender>(addressedMessage.address)) {
                output["to"] = "sender";
            } else if (std::holds_alternative<ToUser>(addressedMessage.address)) {
                output["to"] = std::get<ToUser>(addressedMessage.address).userId;
            }
            
            // Parse the wire message back to JSON to nest it properly
            output["message"] = parseJson(addressedMessage.message);

            Json::StreamWriterBuilder wb;
            wb["indentation"] = ""; // single-line
            std::cout << Json::writeString(wb, output) << "\n";
        }
        
        std::cout.flush();
    }

    return 0;
}
