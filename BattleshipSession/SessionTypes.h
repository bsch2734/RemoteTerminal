#pragma once

#include "GameEntities.h"
#include "Fleet.h"
#include <string>
#include <variant>

using UserId = std::string;

using GameId = std::string;

enum class SessionActionResultError {
    //session-level errors
    shipNotFound,     //the specified shippId is not in this session
    userNotFound,     //the specified userId is not in this session
    unknownAction,    //the action requested is not valid

    //gameplay violations
    wrongPhase,       //action cannot be performed in this game phase
    invalidPlacement, //tried to so something with invalid coordinates
    notYourTurn,      //tried to do something when not allowed to act

    //catch all for engine
    internalError     //generic error
};

enum class SessionActionResultType {
    FireResult,
    ReadyResult,
    PlaceShipResult
};

struct PlaceShipResultData {
    //does not need data, will be enough to see if result was successful
};

struct ReadyResultData {
    //does not need data, will be enough to see if result was successful
};

struct FireResultData {
    bool isHit;
    bool isSunk;
    std::string sunkName;
    int hitId;
};

using SessionActionResultData = std::variant<PlaceShipResultData, ReadyResultData, FireResultData>;

struct SessionActionResult {
    bool success = false;
    SessionActionResultError error = SessionActionResultError::internalError;
    SessionActionResultType type = SessionActionResultType::FireResult;
    SessionActionResultData data = FireResultData();
};

struct UserView {
    UserId userId;
    BoardView boardView;
};

struct SessionSnapshot {
    Phase phase;
    UserId currentUser;
    std::vector<UserView> userViews;
};

struct UserSnapshot {
    Phase phase;
    UserId currentUser;
    UserView userView;
};

struct StartupInfo {
    Phase phase;
    UserId you;
    UserId opponent;
    const Fleet& fleet;
    UserView userView;
    int boardRows;
    int boardCols;
};