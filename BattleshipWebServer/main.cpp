#include "WsGameController.h"
#include <drogon/drogon.h>

int main() {
    auto& app = drogon::app();
    app.addListener("127.0.0.1", 8080);
    app.run();
}
