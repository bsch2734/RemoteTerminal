#include "TicTacToeJsonProtocol.h"

namespace TicTacToe {

Json::Value toJson(const SessionAction& a) {
    Json::Value answer(Json::objectValue);
    answer["type"] = toJson(a.type);
    answer["data"] = toJson(a.data);
    return answer;
}

SessionAction sessionActionFromJson(const Json::Value& v) {
    SessionActionType t = sessionActionTypeFromJson(v["type"]);

    SessionActionData d = MoveData();

    Json::Value actionDataJson = v["data"];

    if (t == SessionActionType::Move)
        d = moveDataFromJson(actionDataJson);
    else if (t == SessionActionType::Rematch)
        d = rematchDataFromJson(actionDataJson);

    return SessionAction{t, d};
}

Json::Value toJson(const MoveData& d) {
    Json::Value answer(Json::objectValue);
    answer["target"] = d.target;
    return answer;
}

MoveData moveDataFromJson(const Json::Value& v) {
    return MoveData{v["target"].asInt()};
}

Json::Value toJson(const RematchData& d) {
    return Json::nullValue;
}

RematchData rematchDataFromJson(const Json::Value& v) {
    return RematchData{};
}

Json::Value toJson(const SessionActionType& t) {
    Json::Value answer(Json::stringValue);
    switch (t) {
        case SessionActionType::Move: {
            answer = "move";
            break;
        }
        case SessionActionType::Rematch: {
            answer = "rematch";
            break;
        }
    }
    return answer;
}

SessionActionType sessionActionTypeFromJson(const Json::Value& v) {
    SessionActionType answer = SessionActionType::Move;

    std::string s = v.asString();
    if (s == "move")
        answer = SessionActionType::Move;
    if (s == "rematch")
        answer = SessionActionType::Rematch;

    return answer;
}

Json::Value toJson(const SessionActionData& d) {
    if (std::holds_alternative<MoveData>(d))
        return toJson(std::get<MoveData>(d));
    if (std::holds_alternative<RematchData>(d))
        return toJson(std::get<RematchData>(d));
    return Json::nullValue;
}

GameId gameIdFromJson(const Json::Value& v) {
    return v.asString();
}

UserId userIdFromJson(const Json::Value& v) {
    return v.asString();
}

Json::Value toJson(const SessionActionResultType& r) {
    Json::Value answer(Json::stringValue);

    switch (r) {
        case SessionActionResultType::MoveResult: {
            answer = "moveresult";
            break;
        }
        case SessionActionResultType::RematchResult: {
            answer = "rematchresult";
            break;
        }
    }

    return answer;
}

Json::Value toJson(const SessionActionResult& r) {
    Json::Value answer(Json::objectValue);

    answer["success"] = r.success;
    answer["error"] = toJson(r.error);
    answer["type"] = toJson(r.type);
    answer["data"] = toJson(r.data);

    return answer;
}

Json::Value toJson(Phase p) {
    Json::Value answer(Json::stringValue);
    switch (p) {
        case Phase::playing: {
            answer = "playing";
            break;
        }
        case Phase::finished: {
            answer = "finished";
            break;
        }
    }
    return answer;
}

Json::Value toJson(const std::string& u) {
    Json::Value answer(Json::stringValue);
    answer = u;
    return answer;
}

Json::Value toJson(const Player& p) {
    Json::Value answer(Json::stringValue);
    switch (p) {
        case Player::none: {
            answer = "none";
            break;
        }
        case Player::one: {
            answer = "one";
            break;
        }
        case Player::two: {
            answer = "two";
            break;
        }
    }
    return answer;
}

Json::Value toJson(const std::array<Player, 9>& board) {
    Json::Value answer(Json::arrayValue);
    for (const Player& p : board)
        answer.append(toJson(p));
    return answer;
}

Json::Value toJson(const SessionActionResultData& s) {
    Json::Value answer(Json::objectValue);
    if (std::holds_alternative<MoveResultData>(s))
        answer = toJson(std::get<MoveResultData>(s));
    else if (std::holds_alternative<RematchResultData>(s))
        answer = toJson(std::get<RematchResultData>(s));
    return answer;
}

Json::Value toJson(const SessionActionResultError& s) {
    Json::Value answer(Json::stringValue);
    switch (s) {
        case SessionActionResultError::none: {
            answer = "none";
            break;
        }
        case SessionActionResultError::internalError: {
            answer = "internalerror";
            break;
        }
        case SessionActionResultError::invalidPlacement: {
            answer = "invalidplacement";
            break;
        }
        case SessionActionResultError::notYourTurn: {
            answer = "notyourturn";
            break;
        }
        case SessionActionResultError::cellTaken: {
            answer = "celltaken";
            break;
        }
        case SessionActionResultError::unknownAction: {
            answer = "unknownaction";
            break;
        }
        case SessionActionResultError::userNotFound: {
            answer = "usernotfound";
            break;
        }
    }
    return answer;
}

Json::Value toJson(const MoveResultData& m) {
    return Json::Value(Json::nullValue);
}

Json::Value toJson(const RematchResultData& r) {
    return Json::Value(Json::nullValue);
}

Json::Value toJson(const RematchRequest& r) {
    Json::Value answer(Json::objectValue);
    answer["user"] = toJson(r.requestingUser);
    return answer;
}

Json::Value toJson(const RematchStart& r) {
    return Json::Value(Json::objectValue);
}

Json::Value toJson(const UserSnapshot& u) {
    Json::Value answer(Json::objectValue);
    answer["phase"] = toJson(u.phase);
    answer["currentturn"] = toJson(u.currentUser);
    answer["board"] = toJson(u.board);
    return answer;
}

Json::Value toJson(const StartupInfo& s) {
    Json::Value answer(Json::objectValue);
    answer["you"] = toJson(s.you);
    answer["opponent"] = toJson(s.opponent);
    answer["yoursymbol"] = toJson(s.yourSymbol);
    answer["opponentsymbol"] = toJson(s.opponentSymbol);
    return answer;
}

Json::Value toJson(const AddUserToGameResult& r) {
    Json::Value answer(Json::objectValue);

    answer["success"] = r.success;
    answer["readytostart"] = r.readyToStart;
    answer["error"] = toJson(r.error);

    if (!r.readyToStart)
        answer["waiting"] = true;

    return answer;
}

Json::Value toJson(const AddUserToGameError& e) {
    Json::Value answer(Json::stringValue);
    switch (e) {
        case AddUserToGameError::none: {
            answer = "none";
            break;
        }
        case AddUserToGameError::userAlreadyInGame: {
            answer = "useralreadyingame";
            break;
        }
        case AddUserToGameError::gameFull: {
            answer = "gamefull";
            break;
        }
    }
    return answer;
}

Json::Value toJson(const OutboundMessage& r) {
    Json::Value answer(Json::objectValue);

    if (std::holds_alternative<UserSnapshot>(r))
        answer["snapshot"] = toJson(std::get<UserSnapshot>(r));
    else if (std::holds_alternative<SessionActionResult>(r))
        answer["actionresult"] = toJson(std::get<SessionActionResult>(r));
    else if (std::holds_alternative<AddUserToGameResult>(r))
        answer = toJson(std::get<AddUserToGameResult>(r));
    else if (std::holds_alternative<StartupInfo>(r))
        answer["startupinfo"] = toJson(std::get<StartupInfo>(r));
    else if (std::holds_alternative<RematchRequest>(r))
        answer["rematchrequest"] = toJson(std::get<RematchRequest>(r));
    else if (std::holds_alternative<RematchStart>(r))
        answer["rematchstart"] = toJson(std::get<RematchStart>(r));

    return answer;
}

JoinRequest joinRequestFromJson(const Json::Value& v) {
    JoinRequest answer{
        userIdFromJson(v["userid"]),
        gameIdFromJson(v["gameid"])
    };
    return answer;
}

Json::Value parseJson(std::string_view s) {
    Json::CharReaderBuilder rb;
    Json::Value root;
    std::string errs;

    std::unique_ptr<Json::CharReader> reader(rb.newCharReader());

    const char* begin = s.data();
    const char* end = s.data() + s.size();

    if (!reader->parse(begin, end, &root, &errs))
        return Json::nullValue;

    return root;
}

ActionRequest actionRequestFromJson(const Json::Value& v) {
    ActionRequest answer{
        gameIdFromJson(v["gameid"]),
        userIdFromJson(v["userid"]),
        sessionActionFromJson(v["sessionaction"])
    };
    return answer;
}

OutboundWireMessage outboundWireMessageFromJson(const Json::Value v) {
    Json::StreamWriterBuilder wb;
    wb["indentation"] = "";

    return Json::writeString(wb, v);
}

} // namespace TicTacToe
