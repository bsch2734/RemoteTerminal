#include "WsGameController.h"
#include <drogon/drogon.h>

int main() {
    auto& app = drogon::app();
    app.addListener("0.0.0.0", 8080);
    app.run();
}
