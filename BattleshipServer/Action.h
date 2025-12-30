#pragma once

#include "coord.h"
#include <variant>


enum class ActionType {
    PlaceShip,
    Ready,
    Fire
};

struct PlaceShipData {
    int shipId;
    int rotation;
    coord posision;
};

struct ReadyData {};

struct FireData {
    coord target;
};

using ActionData = std::variant<PlaceShipData, ReadyData, FireData>;

struct Action {
    ActionType type;
    ActionData data;
};