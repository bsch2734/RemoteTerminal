#include "GameEntities.h"
#include "BattleshipEngine.h"
#include "unordered_set"


BattleshipEngine::BattleshipEngine() :
    _currentPlayer(Player::one),
    _phase(Phase::setup),
    _pOneFleet(baseFleet),
    _pTwoFleet(baseFleet),
    _boardDimensions(10,10)
{
}

const Fleet& BattleshipEngine::getFleetForPlayer(Player p) const{
    return p == Player::one ? _pOneFleet : _pTwoFleet;
}

// only checks the overall status of the fleet
// not the specific result of changing ship s
PlaceShipResult BattleshipEngine::placeShip( Player p, int ID, coord pos, int rotation ) {
    PlaceShipResult r;
    r.success = true;

    //only place in setup phase
    if (phase() != Phase::setup) {
        r.error = PlaceShipError::WrongPhase;
        r.success = false;
        return r;
    }

    Fleet& fleetCopy = getMutableFleetForPlayer(p);
    //find the ship for the player
    Ship* movingShip = nullptr;
    for (Ship& s : fleetCopy.getShips())
        if (s.getID() == ID)
            movingShip = &s;
    if (movingShip == nullptr) {
        r.success = false;
        r.error = PlaceShipError::invalidID;
        return r;
    }

    Ship originalMovingShip = *movingShip;
    movingShip->setPos(pos);
    movingShip->setRotation(rotation);

    auto status = checkFleetStatus(fleetCopy);

    if (status.test((int)FleetStatusBits::unplaced)) {
        r.success = true;
    }
    if (status.test((int)FleetStatusBits::overlapping)) {
        r.success = false;
        r.error = PlaceShipError::OverlapsAnotherShip;
    }
    if (status.test((int)FleetStatusBits::outOfBounds)) {
        r.success = false;
        r.error = PlaceShipError::OutOfBounds;
    }

    if (!r.success) {
        //revert if failed
        movingShip->setPos(originalMovingShip.getPos());
        movingShip->setRotation(originalMovingShip.getRotation());
    }

    return r;
}

//build hitmaps and move to gamplay of other player is ready
bool BattleshipEngine::readyUp(Player p) {
    auto status = checkFleetStatus(getFleetForPlayer(p));
    if (status.any())
        return false;

    (p == Player::one ? p1IsReady : p2IsReady) = true;
    if (p1IsReady && p2IsReady)
        _phase = Phase::playing;

    Fleet& fleet = getMutableFleetForPlayer(p);
    auto& hitMap = getHitmapForPlayer(p);
    for (Ship& s : fleet.getShips()) {
        for (coord c : s.getCoords()) {
            coord transformed = c.applyTransform(s.getPos(), s.getRotation());
            hitMap[transformed] = s.getID();
        }
    }

    return true;
}

// --- Gameplay ---
FireResult BattleshipEngine::fire(Player p, coord target) {
    FireResult answer;
    

    return FireResult::miss;
}

// --- Queries ---
Phase BattleshipEngine::phase() const {
    return _phase;
}

Player BattleshipEngine::getWinner() const{
    if (_phase != Phase::finished)
        return Player::none;
    if (_pOneFleet.isDefeated())
        return Player::two;
    else
        return Player::one;
}

Player BattleshipEngine::currentTurn() const {
    return _currentPlayer;
}

Fleet& BattleshipEngine::getMutableFleetForPlayer(Player p) {
    return p == Player::one ? _pOneFleet : _pTwoFleet;
}

//default fleet for normal game
Fleet BattleshipEngine::baseFleet{
    {
        Ship::carrier,
        Ship::battleship,
        Ship::destroyer,
        Ship::sub,
        Ship::pt,
    }
};

std::unordered_map<coord, int>& BattleshipEngine::getHitmapForPlayer(Player p) {
    return p == Player::one? p1HitMap : p2HitMap;
}

std::bitset<8> BattleshipEngine::checkFleetStatus(Fleet f) {
    std::bitset<8> answer;
    std::unordered_set<coord> occupied;
    for (Ship s : f.getShips()) {
        if (!s.getPos().isUnspecified()) { //only check ships that are placed
            for (coord c : s.getCoords()) {
                coord transformed = c.applyTransform(s.getPos(), s.getRotation());
                if (transformed.d >= _boardDimensions.first
                    || transformed.o >= _boardDimensions.second
                    || transformed.d < 0
                    || transformed.o < 0) {
                    answer.set((int)FleetStatusBits::outOfBounds, true);
                    continue;//cannot be out of bounds and overlapping
                }
                size_t occupiedSize = occupied.size();
                occupied.insert(transformed);
                if (occupiedSize == occupied.size())
                    answer.set((int)FleetStatusBits::overlapping, true);
            }
        }
        else {
            answer.set((int)FleetStatusBits::unplaced, true);
        }
    }
    return answer;
}

//BoardView ownBoard(Player p) const;
//BoardView opponentBoard(Player p) const;