#pragma once

#include "BattleshipEngine.h"
#include "Action.h"

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
    std::set<UserView> userViews;
};

class BattleshipSession {
public:
    BattleshipSession(std::string id, std::string playerOneId, std::string playerTwoId);

    std::string id() const;

    bool isFinished() const;

    SessionResult handleAction(std::string user, const Action& action);

    SessionSnapshot getSnapshot();

private:
    // Helpers
    Player playerFor(std::string user) const;

    //SessionResult makeError(SessionError error) const;
    //SessionResult makeResultFromEngine(FireResult fireResult);
    //SessionResult makeResultFromEngine(PlaceShipResult placeResult);
    //SessionResult makeResultNoOp(); // for Ready, etc.

    //PlayerView buildView(Player p) const;

    SessionResult handlePlaceShip(Player p, const Action& a);
    SessionResult handleFire(Player p, const Action& a);
    SessionResult handleReady(Player p);

private:
    std::string _sessionId;
    std::map<std::string, Player> _userToPlyaerMap;
    std::map<Player, std::string> _playerToUserMap;

    BattleshipEngine _engine;
};
