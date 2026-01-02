#include "GameEntities.h"
#include "BattleshipEngine.h"
#include "unordered_set"


BattleshipEngine::BattleshipEngine() :
    _currentPlayer(Player::none),
    _phase(Phase::setup),
    _pOneFleet(getBaseFleet()),
    _pTwoFleet(getBaseFleet()),
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
ReadyUpResult BattleshipEngine::readyUp(Player p) {
    ReadyUpResult answer;
    auto status = checkFleetStatus(getFleetForPlayer(p));
    if (status.any()) {
        answer.succes = false;

        if (status.test((int)FleetStatusBits::unplaced))
            answer.error = ReadyUpError::fleetNotPlaced;
        else if (status.test((int)FleetStatusBits::overlapping)) 
            answer.error = ReadyUpError::fleetPlacementInvalid;
        else if (status.test((int)FleetStatusBits::outOfBounds)) 
            answer.error = ReadyUpError::fleetPlacementInvalid;

        return answer;
    }
    (p == Player::one ? p1IsReady : p2IsReady) = true;
    if (p1IsReady && p2IsReady) {
        _phase = Phase::playing;
        _currentPlayer = Player::one;
    }

    answer.succes = true;
    return answer;
}

// --- Gameplay ---
FireResult BattleshipEngine::fire(Player p, coord target) {
    FireResult answer;
    
    if (_currentPlayer != p) {
        answer.success = false;
        answer.error = FireError::notYourTurn;
        return answer;
    }

    if(    target.d >= _boardDimensions.first 
        || target.o >= _boardDimensions.second
        || target.d < 0
        || target.o < 0){
        answer.success = false;
        answer.error = FireError::outOfBounds;
        return answer;
    }

    auto& f = getMutableFleetForPlayer(oponent(p));
    auto r = f.hitFleet(target);

    if (r.success) {
        answer.success = true;
        answer.isHit = true;
        getHitsForPlayer(p).insert(target);
        answer.isSink = r.sunk;
        answer.hitId = r.hitID;
    }
    else {
        if(r.error != Fleet::hitFleetError::coordAlreadyHit) //not a true miss if this coord was hit before
            getMissesForPlayer(p).insert(target);
        answer.success = true;
        answer.isHit = false;
    }

    if (f.isDefeated()) {
        _phase = Phase::finished;
        _currentPlayer = Player::none;
    }
    else
        if (answer.success)
            _currentPlayer = oponent(p);

    return answer;
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

std::string BattleshipEngine::nameForId(int id) const {
    for (const Ship& s : getFleetForPlayer(Player::one).getShips())
        if (s.getID() == id)
            return s.getName();
    for (const Ship& s : getFleetForPlayer(Player::two).getShips())
        if (s.getID() == id)
            return s.getName();

    return "";
}

const std::set<coord>& BattleshipEngine::getHitsForPlayer(Player p) const{
    if (p == Player::one)
        return _p1Hits;
    return _p2Hits; //should never see player::none but this may cause issues if it does
}

const std::set<coord>& BattleshipEngine::getMissesForPlayer(Player p) const{
    if (p == Player::one)
        return _p1Misses;
    return _p2Misses; //should never see player::none but this may cause issues if it does
}

BoardView BattleshipEngine::boardViewForPlayer(Player p) const{
    BoardView b;
    b.ownGrid = ownGrid(p);
    b.oponentGrid = opponentGrid(p);
    return b;
}

GridView BattleshipEngine::ownGrid(Player p) const {
    std::map<coord, squareStates> occupied;
    //layer from bottom to top so only top is visible
    const Fleet& f = getFleetForPlayer(p);
    for (const Ship& s : f.getShips())
        for (const coord& c : s.getCoords())
            occupied[c.applyTransform(s.getPos(), s.getRotation())] = squareStates::ship;
    for (const auto& c : getMissesForPlayer(oponent(p)))
        occupied[c] = squareStates::miss;
    for (const auto& c : getHitsForPlayer(oponent(p)))
        occupied[c] = squareStates::hit;
    return GridView(occupied);
}

GridView BattleshipEngine::opponentGrid(Player p) const {
    std::map<coord, squareStates> occupied;
    //layer from bottom to top so only top is visible
    for (const auto& c : getMissesForPlayer(p))
        occupied[c] = squareStates::miss;
    for (const auto& c : getHitsForPlayer(p))
        occupied[c] = squareStates::hit;
    return GridView(occupied);
}

Fleet& BattleshipEngine::getMutableFleetForPlayer(Player p) {
    return p == Player::one ? _pOneFleet : _pTwoFleet;
}

std::set<coord>& BattleshipEngine::getHitsForPlayer(Player p) {
    if (p == Player::one)
        return _p1Hits;
    return _p2Hits; //should never see player::none but this may cause issues if it does
}

std::set<coord>& BattleshipEngine::getMissesForPlayer(Player p) {
    if (p == Player::one)
        return _p1Misses;
    return _p2Misses; //should never see player::none but this may cause issues if it does
}

//default fleet for normal game
Fleet const& BattleshipEngine::getBaseFleet() {
    static Fleet baseFleet{
        {
            Ship::carrier,
            Ship::battleship,
            Ship::destroyer,
            Ship::sub,
            Ship::pt,
        }
    };
    return baseFleet;
}

std::bitset<8> BattleshipEngine::checkFleetStatus(Fleet f) {
    std::bitset<8> answer;
    std::unordered_set<coord> occupied;
    for (Ship s : f.getShips()) {
        if (s.isPlaced()) { //only check ships that are placed
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
