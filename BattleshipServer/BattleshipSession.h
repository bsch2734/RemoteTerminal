#pragma once

#include "BattleshipEngine.h"
#include "Action.h"

enum class SessionError {
    //session-level errors
    userNotFound,
    unknownAction,

    //gameplay violations
    wrongPhase,
    outOfBounds,
    notYourTurn,

    //catch all for engine
    internalError
};

struct SessionResult {
    bool success;
    SessionError error;
};

class BattleshipSession {
public:
    BattleshipSession(std::string id, std::string playerOneId, std::string playerTwoId);

    std::string id() const;

    bool isFinished() const;

    SessionResult handleAction(std::string user, const Action& action);



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

    BattleshipEngine _engine;
};
