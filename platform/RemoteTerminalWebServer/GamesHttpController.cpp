#include "GamesHttpController.h"
#include <json/json.h>

void GamesHttpController::listGames(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    Json::Value games(Json::arrayValue);

    // Naval Battle
    Json::Value battleship;
    battleship["id"] = "navalbattle";
    battleship["name"] = "Naval Battle";
    battleship["description"] = "Classic naval combat strategy game";
    battleship["route"] = "com.titohq.navalbattle";
    battleship["url"] = "/navalbattle/";
    battleship["icon"] = "\u2693";
    battleship["minPlayers"] = 2;
    battleship["maxPlayers"] = 2;
    games.append(battleship);

    // Tic Tac Toe
    Json::Value tictactoe;
    tictactoe["id"] = "tictactoe";
    tictactoe["name"] = "Tic Tac Toe";
    tictactoe["description"] = "Classic X and O game";
    tictactoe["route"] = "com.titohq.tictactoe";
    tictactoe["url"] = "/tictactoe/";
    tictactoe["icon"] = "\u274C";
    tictactoe["minPlayers"] = 2;
    tictactoe["maxPlayers"] = 2;
    games.append(tictactoe);

    Json::Value response;
    response["games"] = games;

    auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
    resp->addHeader("Access-Control-Allow-Origin", "*");
    callback(resp);
}
