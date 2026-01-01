#pragma once

enum class SessionResultError {
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

enum class SessionResultType {
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

using SessionResultData = std::variant<PlaceShipResultData, ReadyResultData, FireResultData>;

struct SessionResult {
    bool success = false;
    SessionResultError error = SessionResultError::internalError;
    SessionResultType type = SessionResultType::FireResult;
    SessionResultData data = FireResultData();
};

struct UserView {
    std::string userId;
    BoardView boardView;
};

struct SessionSnapshot {
    Phase phase;
    std::string currentUser;
    std::vector<UserView> userViews;
};