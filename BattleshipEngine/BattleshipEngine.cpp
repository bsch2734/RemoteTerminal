#include "BattleshipEngine.h"
#include "GameEntities.h"
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

    // Validate the placement
    auto validation = validatePlacement(p, ID, pos, rotation);
    
    if (!validation.valid) {
        r.success = false;
        r.error = validation.error;
        return r;
    }

    // Validation passed, now actually place the ship
    Fleet& fleet = getMutableFleetForPlayer(p);
    for (Ship& s : fleet.getShips()) {
        if (s.getID() == ID) {
            s.setPos(pos);
            s.setRotation(rotation);
            break;
        }
    }

    return r;
}

ValidatePlacementResult BattleshipEngine::validatePlacement(Player p, int ID, coord pos, int rotation) const {
    ValidatePlacementResult r;
    r.valid = true;

    const Fleet& fleet = getFleetForPlayer(p);
    
    // Find the ship
    const Ship* targetShip = nullptr;
    for (const Ship& s : fleet.getShips())
        if (s.getID() == ID)
            targetShip = &s;
    
    if (targetShip == nullptr) {
        r.valid = false;
        r.error = PlaceShipError::invalidID;
        return r;
    }
    
    // Collect coords of other placed ships
    std::unordered_set<coord> occupied;
    for (const Ship& s : fleet.getShips()) {
        if (s.getID() == ID)
            continue;
        if (s.isPlaced()) {
            for (const coord& c : s.getCoords()) {
                occupied.insert(c.applyTransform(s.getPos(), s.getRotation()));
            }
        }
    }
    
    // Calculate placement coords and check validity
    for (const coord& c : targetShip->getCoords()) {
        coord transformed = c.applyTransform(pos, rotation);
        r.coords.insert(transformed);
        
        // Check out of bounds
        if (transformed.d < 0 || transformed.d >= _boardDimensions.first ||
            transformed.o < 0 || transformed.o >= _boardDimensions.second) {
            r.valid = false;
            r.error = PlaceShipError::OutOfBounds;
        }
        
        // Check overlap (only set error if not already invalid)
        if (r.valid && occupied.find(transformed) != occupied.end()) {
            r.valid = false;
            r.error = PlaceShipError::OverlapsAnotherShip;
        }
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

bool BattleshipEngine::isPlayerReady(Player p) {
    if (p == Player::one)
        return p1IsReady;
    if (p == Player::two)
        return p2IsReady;
    return false;
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

int BattleshipEngine::boardRows() {
    return _boardDimensions.first;
}

int BattleshipEngine::boardCols() {
    return _boardDimensions.second;
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
    b.opponentGrid = opponentGrid(p);
    return b;
}

GridView BattleshipEngine::ownGrid(Player p) const {
    std::map<coord, SquareState> occupied;
    //layer from bottom to top so only top is visible
    const Fleet& f = getFleetForPlayer(p);
    for (const Ship& s : f.getShips())
        if(s.isPlaced())
            for (const coord& c : s.getCoords())
                occupied[c.applyTransform(s.getPos(), s.getRotation())] = SquareState::ship;
    for (const auto& c : getMissesForPlayer(oponent(p)))
        occupied[c] = SquareState::miss;
    for (const auto& c : getHitsForPlayer(oponent(p)))
        occupied[c] = SquareState::hit;
    return GridView(occupied);
}

GridView BattleshipEngine::opponentGrid(Player p) const {
    std::map<coord, SquareState> occupied;
    //layer from bottom to top so only top is visible
    for (const auto& c : getMissesForPlayer(p))
        occupied[c] = SquareState::miss;
    for (const auto& c : getHitsForPlayer(p))
        occupied[c] = SquareState::hit;
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
