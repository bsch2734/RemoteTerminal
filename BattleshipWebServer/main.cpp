#include "WsGameController.h"
#include <drogon/drogon.h>

int main(int argc, char* argv[]) {
    int port = argc >= 2 ? std::stoi(argv[1]) : 8080; //allow port selection as runtime arg, default to 8080
    auto& app = drogon::app();
    app.addListener("127.0.0.1", port);
    app.run();
}
