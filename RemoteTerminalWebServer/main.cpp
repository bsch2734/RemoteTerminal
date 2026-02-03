#include "RemoteTerminalWebSocketController.h"
#include "RemoteTerminalEndpointRegistry.h"
#include "BattleshipEndpoint.h"
#include <drogon/drogon.h>

int main(int argc, char* argv[]) {
    RemoteTerminalEndpointRegistry registry;
    BattleshipEndpoint* battleshipEndpoint = new BattleshipEndpoint();
    registry.registerEndpoint("/ws/" + battleshipEndpoint->routePath(), battleshipEndpoint);

	getRemoteTerminalWebSocketManager().setEndpointRegistry(&registry);

    int port = argc >= 2 ? std::stoi(argv[1]) : 8080; //allow port selection as runtime arg, default to 8080
    auto& app = drogon::app();
    app.addListener("127.0.0.1", port);
	app.setThreadNum(1); //single-threaded to avoid concurrency issues in game logic
    app.run();
}
