#pragma once

#include "coord.h"
#include <variant>


enum class SessionActionType {
    PlaceShip,
    Ready,
    Fire,
    CheckPlacement
};

struct PlaceShipData {
    int shipId;
    int rotation;
    coord position;
};

struct ReadyData {};

struct FireData {
    coord target;
};

// Note: CheckPlacement uses PlaceShipData directly, not in the variant
using SessionActionData = std::variant<PlaceShipData, ReadyData, FireData>;

struct SessionAction {
    SessionActionType type;
    SessionActionData data;
};