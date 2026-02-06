#pragma once

#include "CoreTypes.h"
#include "TicTacToeEntities.h"
#include <string>
#include <variant>
#include <array>
#include <vector>

namespace TicTacToe {

enum class SessionActionResultError {
    none,
    userNotFound,
    unknownAction,
    invalidPlacement,
    cellTaken,
    notYourTurn,
    internalError
};

enum class SessionActionResultType {
    MoveResult
};

enum class SessionActionType {
    Move
};

struct MoveData {
    int target;
};

struct MoveResultData {};

using SessionActionData = std::variant<MoveData>;

struct SessionAction {
    SessionActionType type;
    SessionActionData data;
};

using SessionActionResultData = std::variant<MoveResultData>;

struct SessionActionResult {
    bool success = false;
    SessionActionResultError error = SessionActionResultError::none;
    SessionActionResultType type = SessionActionResultType::MoveResult;
    SessionActionResultData data = MoveResultData();
};

struct UserSnapshot {
    Phase phase;
    UserId currentUser;
    std::array<Player, 9> board;
};

struct StartupInfo {
    UserId you;
    UserId opponent;
    Player yourSymbol;
    Player opponentSymbol;
};

enum class AddUserToGameError {
    none,
    userAlreadyInGame,
    gameFull
};

struct AddUserToGameResult {
    bool success = false;
    bool readyToStart = false;
    AddUserToGameError error = AddUserToGameError::none;
};

using OutboundMessage = std::variant<UserSnapshot, SessionActionResult, AddUserToGameResult, StartupInfo>;

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

} // namespace TicTacToe
