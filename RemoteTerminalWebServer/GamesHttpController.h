#pragma once

#include <drogon/HttpController.h>

class GamesHttpController : public drogon::HttpController<GamesHttpController> {
public:
    METHOD_LIST_BEGIN
        ADD_METHOD_TO(GamesHttpController::listGames, "/api/games", drogon::Get, "drogon::LocalHostFilter");
    METHOD_LIST_END

    void listGames(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};
