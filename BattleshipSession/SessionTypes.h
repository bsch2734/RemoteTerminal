#pragma once

#include "CoreTypes.h"
#include "GameEntities.h"
#include "Fleet.h"
#include "Action.h"
#include <string>
#include <variant>

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
    PlaceShipResult,
    CheckPlacementResult
};

struct PlaceShipResultData {
    //does not need data, will be enough to see if result was successful
};

struct ReadyResultData {
    //does not need data, will be enough to see if result was successful
};

struct FireResultData {
    bool isHit = false;
    bool isSunk = false;
    std::string sunkName = "";
    int hitId = 0;
};

struct CheckPlacementResultData {
    bool valid = false;
    std::set<coord> coords;
};

using SessionActionResultData = std::variant<PlaceShipResultData, ReadyResultData, FireResultData, CheckPlacementResultData>;

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

struct UserSnapshot {
    Phase phase;
    UserId currentUser;
    UserView userView;
    bool youReady;
    bool opponentReady;
};

struct StartupInfo {
    Phase phase;
    UserId you;
    UserId opponent;
    GameId gameId;
    const Fleet& fleet;
    UserView userView;
    int boardRows;
    int boardCols;
};

enum class AddUserToGameError {
    userAlreadyInGame,
    gameFull
};

struct AddUserToGameResult {
    bool success = false;
    bool readyToStart = false;
    AddUserToGameError error;
};

using OutboundMessage = std::variant<UserSnapshot, StartupInfo, SessionActionResult, AddUserToGameResult>;

struct AddressedMessage {
    OutboundMessageReciever address;
    OutboundMessage message;
};

class AddressedMessageBundle {
public:
    AddressedMessageBundle& addMessage(const OutboundMessageReciever& reciever, const OutboundMessage& message) {
        messages.emplace_back(AddressedMessage{reciever, message});
        return *this;
    }

    AddressedMessageBundle& addMessageBundle(const AddressedMessageBundle& bundle) {
        for (const auto& msg : bundle.messages) {
            messages.emplace_back(msg);
        }
        return *this;
	}

    auto begin() { return messages.begin(); }
    auto end() { return messages.end(); }

    auto begin() const { return messages.begin(); }
    auto end() const { return messages.end(); }
	
private:
	std::vector<AddressedMessage> messages;
};

struct JoinRequest {
    UserId userId;
    GameId gameId;
};

struct ActionRequest {
    GameId gameId;
    UserId userId;
    SessionAction action;
};
