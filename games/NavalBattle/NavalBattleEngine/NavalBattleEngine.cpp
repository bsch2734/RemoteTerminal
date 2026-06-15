#include "NavalBattleEngine.h"
#include "GameEntities.h"
#include <unordered_set>

using namespace NavalBattle;

template<class T>
inline constexpr bool always_false = false;

NavalBattleEngine::NavalBattleEngine(GameMode mode) :
    _currentPlayer(Player::none),
    _phase(Phase::setup),
    _p1Data(createPlayerDataForGameMode(mode)),
    _p2Data(createPlayerDataForGameMode(mode)),
    _pNoneData(Fleet()),
    _boardDimensions(getBoardDimensionsForMode(mode))
{
}

const Fleet& NavalBattleEngine::getFleetForPlayer(Player p) const{
	return const_cast<NavalBattleEngine*>(this)->getFleetForPlayer(p);
}

Fleet& NavalBattleEngine::getFleetForPlayer(Player p) {
	return getDataForPlayer(p).fleet;
}

FleetView NavalBattle::NavalBattleEngine::getViewOfOwnFleet(Player viewer) const {
    FleetView answer;
    const auto& fleet = getFleetForPlayer(viewer);
	const auto& ships = fleet.getShips();

    for (const Ship& s : ships) {
        ShipView sv;
        sv.id = s.getId();
        sv.name = s.getName();
        sv.shape = s.getCoords();
        sv.pos = s.getPos();
        sv.rotation = s.getRotation();
        sv.isSunk = s.isSunk();
		for (const VehicleAbility& a : s.getAbilities())
            sv.abilities.push_back(a);
        answer.ships.push_back(sv);
    }
	const auto& planes = fleet.getPlanes();
    for (const Plane& p : planes) {
        PlaneView pv;
		pv.id = p.getId();
        pv.name = p.getName();
        pv.pos = p.getPos();
        pv.isOnShip = p.isOnShip();
        pv.isDestroyed = p.isDestroyed();
        for (const VehicleAbility& a : p.getAbilities())
            pv.abilities.push_back(a);
		answer.planes.push_back(pv);
    }

	return answer;
}

FleetView NavalBattleEngine::getViewOfOpponentFleet(Player viewer) const {
    FleetView answer;
    const auto& fleet = getFleetForPlayer(opponent(viewer));
    const auto& ships = fleet.getShips();
    for (const Ship& s : ships) {
        ShipView sv;
        sv.id = s.getId();
        sv.name = s.getName();
        sv.shape = s.getCoords();
        sv.pos = std::nullopt;
        sv.rotation = std::nullopt;
        sv.isSunk = s.isSunk();
        for (const VehicleAbility& a : s.getAbilities())
            sv.abilities.push_back(a);
        answer.ships.push_back(sv);
    }
    const auto& planes = fleet.getPlanes();
    for (const Plane& p : planes) {
        PlaneView pv;
        pv.id = p.getId();
        pv.name = p.getName();
        pv.pos = std::nullopt;
        pv.isOnShip = p.isOnShip();
        pv.isDestroyed = p.isDestroyed();
        for (const VehicleAbility& a : p.getAbilities())
            pv.abilities.push_back(a);
        answer.planes.push_back(pv);
    }
    return answer;
}

// only checks the overall status of the fleet
// not the specific result of changing ship s
PlaceShipResult NavalBattleEngine::placeShip( Player p, int ID, coord pos, int rotation ) {
    PlaceShipResult r;
    r.success = true;

    //only place in setup phase
    if (phase() != Phase::setup) {
        r.error = PlaceShipError::WrongPhase;
        r.success = false;
        return r;
    }

    // Validate the placement
    auto validation = validateShipPlacement(p, ID, pos, rotation);
    
    if (!validation.valid) {
        r.success = false;
        r.error = validation.error;
        return r;
    }

    // Validation passed, now actually place the ship
    Fleet& fleet = getFleetForPlayer(p);
    fleet.placeShip(ID, pos, rotation);

    return r;
}

ValidateShipPlacementResult NavalBattleEngine::validateShipPlacement(Player p, int ID, coord pos, int rotation) const {
    ValidateShipPlacementResult r;
    r.valid = true;

    const Fleet& fleet = getFleetForPlayer(p);
    
    // Find the ship
	const Ship* targetShip = fleet.getShipById(ID);
    
    if (targetShip == nullptr) {
        r.valid = false;
        r.error = PlaceShipError::invalidID;
        return r;
    }
    
    // Collect coords of other placed ships
    std::unordered_set<coord> occupied;
    for (const Ship& s : fleet.getShips())
        if (s.getId() == ID)
            continue;
        else if (s.isPlaced()) 
            for (const coord& c : s.getCoords()) 
                occupied.insert(c.applyTransform(s.getPos(), s.getRotation()));
    
    // Calculate placement coords and check validity
    for (const coord& c : targetShip->getCoords()) {
        coord transformed = c.applyTransform(pos, rotation);
        
        // Check out of bounds - don't include invalid coords in result
		if (!isValidCoord(transformed)) {
             r.valid = false;
             r.error = PlaceShipError::OutOfBounds;
             continue;//cannot be out of bounds and overlapping
        }
        
        r.coords.insert(transformed);
        
        // Check overlap
        if (r.valid && occupied.find(transformed) != occupied.end()) {
            r.valid = false;
            r.error = PlaceShipError::OverlapsAnotherShip;
        }
    }
    
    return r;
}

ValidateAbilityResult NavalBattleEngine::validateAbility(VehicleAbilityActionData data, const AbilityContext& ctx) const {
	ValidateAbilityResult answer;
	std::visit([this, &answer, &ctx](auto&& arg) {
		using T = std::decay_t<decltype(arg)>;
		if constexpr (std::is_same_v<T, TorpedoData>)
			answer = validateTorpedoData(arg);
		else if constexpr (std::is_same_v<T, ExocetData>)
			answer = validateExocetData(arg);
		else if constexpr (std::is_same_v<T, ApacheData>)
			answer = validateApacheData(arg);
		else if constexpr (std::is_same_v<T, TomahawkData>)
			answer = validateTomahawkData(arg);
		else if constexpr (std::is_same_v<T, RelocateData>)
			answer = validateRelocateData(arg);
		else if constexpr (std::is_same_v<T, ScanData>)
			answer = validateScanData(arg);
		else if constexpr (std::is_same_v<T, RevealData>)
			answer = validateRevealData(arg, ctx);
		else
			static_assert(always_false<T>, "non-exhaustive visitor!");
	}, data);
	return answer;
}

PlacePlaneResult NavalBattleEngine::placePlane(Player p, int ID, coord pos) {
    PlacePlaneResult r;
    r.success = true;

    // Only place in setup phase
    if (phase() != Phase::setup) {
        r.error = PlacePlaneError::WrongPhase;
        r.success = false;
        return r;
    }

    // Validate the placement
    auto validation = validatePlanePlacement(p, ID, pos);

    if (!validation.valid) {
        r.success = false;
        r.error = validation.error;
        return r;
    }

    // Validation passed, place the plane
    Fleet& fleet = getFleetForPlayer(p);
    fleet.placePlane(ID, pos);

    return r;
}

ValidatePlanePlacementResult NavalBattleEngine::validatePlanePlacement(Player p, int ID, coord pos) const {
    ValidatePlanePlacementResult r;
    r.valid = true;
    r.position = pos;

    const Fleet& fleet = getFleetForPlayer(p);

    // Find the plane
    const Plane* targetPlane = fleet.getPlaneById(ID);

    if (targetPlane == nullptr) {
        r.valid = false;
        r.error = PlacePlaneError::invalidID;
        return r;
    }

    // Collect squares that can hold planes (carrier squares)
    std::unordered_set<coord> carrierSquares;
    for (const Ship& s : fleet.getShips()) 
        if (s.isPlaced() && s.canHoldPlanes()) 
            for (const coord& c : s.getCoords()) 
                carrierSquares.insert(c.applyTransform(s.getPos(), s.getRotation()));

    // Check if target position is on a carrier
    if (carrierSquares.find(pos) == carrierSquares.end()) {
        r.valid = false;
        r.error = PlacePlaneError::NotOnCarrier;
        return r;
    }

    // Check for overlap with other planes
    for (const Plane& plane : fleet.getPlanes()) {
        if (plane.getId() == ID)
            continue;
        if (plane.isPlaced() && plane.getPos() == pos) {
            r.valid = false;
            r.error = PlacePlaneError::OverlapsAnotherPlane;
            return r;
        }
    }

    return r;
}

ActivateAbilityResult NavalBattleEngine::activateAbility(Player p, int shipId, const VehicleAbilityAction& VehicleAbilityAction) {
    ActivateAbilityResult answer;

    ActivateAbilityResultError e;
    
	e = playerMayActivateAbility(p, shipId, VehicleAbilityAction.type);
	if (e != ActivateAbilityResultError::none) {
		answer.success = false;
		answer.error = e;
		return answer;
	}

	AbilityContext ctx{ shipId };
	ValidateAbilityResult validation = validateAbility(VehicleAbilityAction.data, ctx);
	if (validation.error != ActivateAbilityResultError::none) {
		answer.success = false;
		answer.error = validation.error;
		return answer;
	}

	answer.success = true;

	answer.data = executeAbilityPlan(p, validation.plan);

	getDataForPlayer(p).fleet.useShipAbility(shipId, VehicleAbilityAction.type);
	advanceTurn();

	return answer;
}

FireResult NavalBattleEngine::fireAntiAircraft(Player p, coord target) {
    FireResult answer;

    if (_currentPlayer != p) {
        answer.success = false;
        answer.error = FireError::notYourTurn;
        return answer;
    }

    if (!isValidCoord(target)) {
        answer.success = false;
        answer.error = FireError::outOfBounds;
        return answer;
    }

    answer.success = true;

    // Try to hit a plane at this coordinate
    Fleet::hitFleetPlanesResult r = getFleetForPlayer(opponent(p)).hitFleetPlanesInAir(target);    

    if (r.success) {
        answer.isHit = true;
        answer.isSink = r.destroyed;
        answer.hitId = r.hitID;
    }
    else
        answer.isHit = false;
    if (answer.success)
        advanceTurn();
    
    return answer;
}


ActivateAbilityResultData NavalBattleEngine::executeTorpedoPlan(Player p, TorpedoPlan plan) {
    TorpedoResultData answer;
    TorpedoResultData data;

    coord currentPos = plan.startPoint;
    std::function<void()> incrementPos;

	switch (plan.direction) {
        case TorpedoPlan::TorpedoDirection::up:
            incrementPos = [&currentPos]() {currentPos.d--; };
            break;
        case TorpedoPlan::TorpedoDirection::down:
            incrementPos = [&currentPos]() {currentPos.d++; };
            break;
        case TorpedoPlan::TorpedoDirection::left:
            incrementPos = [&currentPos]() {currentPos.o--; };
            break;
        case TorpedoPlan::TorpedoDirection::right:
            incrementPos = [&currentPos]() {currentPos.o++; };
            break;
    }

    bool isHit = false;
    do {
        if (checkCoord(opponent(p), currentPos)) {
            hitCoord(opponent(p), currentPos);
            isHit = true;
            break;
        }
        else {
            getDataForPlayer(p).revealedMisses.insert(currentPos);
        }
        incrementPos();
    } while (isValidCoord(currentPos));

    answer.isHit = isHit;
    return answer;
}

NavalBattleEngine::BulkFireValidationResult NavalBattleEngine::validateBulkFireTargets(BulkFireData data) const {
	BulkFireValidationResult result;
	for (coord c : data.targets)
	    if (isValidCoord(c))
            result.targets.insert(c);
	    else
	        result.error = ActivateAbilityResultError::outOfBounds;
	return result;
}

ActivateAbilityResultData NavalBattle::NavalBattleEngine::executeAbilityPlan(Player p, const AbilityPlan& plan) {
	return std::visit([this, p](auto&& arg) {
		using T = std::decay_t<decltype(arg)>;
		if constexpr (std::is_same_v<T, TorpedoPlan>)
			return executeTorpedoPlan(p, arg);
		else if constexpr (std::is_same_v<T, ExocetPlan>)
			return executeExocetPlan(p, arg);
		else if constexpr (std::is_same_v<T, ApachePlan>)
			return executeApachePlan(p, arg);
		else if constexpr (std::is_same_v<T, TomahawkPlan>)
			return executeTomahawkPlan(p, arg);
		else if constexpr (std::is_same_v<T, RelocatePlan>)
			return executeRelocatePlan(p, arg);
		else if constexpr (std::is_same_v<T, ScanPlan>)
			return executeScanPlan(p, arg);
		else if constexpr (std::is_same_v<T, RevealPlan>)
			return executeRevealPlan(p, arg);
		else
			static_assert(always_false<T>, "Non-exhaustive visitor!");
		}, plan);
}

bool NavalBattleEngine::executeBulkFireTargets(Player p, const std::set<coord>& targets) {
    bool isHit = false;
    for (const coord& c : targets)
        if (hitCoord(opponent(p), c).success)
            isHit = true;
    return isHit;
}

ActivateAbilityResultData NavalBattleEngine::executeExocetPlan(Player p, ExocetPlan plan) {
    return ExocetResultData{ executeBulkFireTargets(p, plan.targets) };
}

ActivateAbilityResultData NavalBattleEngine::executeApachePlan(Player p, ApachePlan plan) {
    return ApacheResultData{ executeBulkFireTargets(p, plan.targets) };
}

ActivateAbilityResultData NavalBattleEngine::executeTomahawkPlan(Player p, TomahawkPlan plan) {
    return TomahawkResultData{ executeBulkFireTargets(p, plan.targets) };
}

ActivateAbilityResultData NavalBattleEngine::executeRelocatePlan(Player p, RelocatePlan plan) {
	RelocateResultData answer;
	answer.shipId = plan.shipId;

	Fleet& f = getDataForPlayer(p).fleet;
	f.placeVehicle(plan.shipId, plan.target);
	f.vehicleIsOnShip(plan.shipId, plan.willBeOnShip);

	return answer;
}

ActivateAbilityResultData NavalBattleEngine::executeScanPlan(Player p, ScanPlan plan) {
    ScanResultData answer;
    answer.isFound = false;

    for (const coord& c : plan.targets)
        if (getDataForPlayer(opponent(p)).fleet.wouldBeHit(c)) {
            getDataForPlayer(p).scansWithHits.insert(plan.targets);

            answer.isFound = true;
            return answer;
        }

    //scan is clear
    //this is the same as guessing/ missing all scanned squares 
    getDataForPlayer(p).revealedMisses.insert(plan.targets.begin(), plan.targets.end());

    return answer;
}

ActivateAbilityResultData NavalBattleEngine::executeRevealPlan(Player p, RevealPlan plan) {
    RevealResultData answer;

    for (const coord& c : plan.targets)
        if (checkCoord(opponent(p), c)) { // c would be a hit
            getDataForPlayer(p).revealedHits.insert(c);
            answer.hitsRevealed.insert(c);
        }
        else
            getDataForPlayer(p).revealedMisses.insert(c);

    return answer;
}

ValidateAbilityResult NavalBattleEngine::validateTorpedoData(TorpedoData d) const {
    ValidateAbilityResult answer;
    TorpedoPlan plan;

	plan.startPoint = d.startPoint;

    if (d.firingPattern == TorpedoData::FiringPattern::vertical) {
        if (d.startPoint.d != 0 && d.startPoint.d != boardRows() - 1)
            answer.error = ActivateAbilityResultError::outOfBounds;
		plan.direction = (d.startPoint.d == 0) ? TorpedoPlan::TorpedoDirection::down : TorpedoPlan::TorpedoDirection::up;
    }
    else if (d.firingPattern == TorpedoData::FiringPattern::horizontal) {
        if (d.startPoint.o != 0 && d.startPoint.o != boardCols() - 1)
            answer.error = ActivateAbilityResultError::outOfBounds;
		plan.direction = (d.startPoint.o == 0) ? TorpedoPlan::TorpedoDirection::right : TorpedoPlan::TorpedoDirection::left;
    }
    else 
		answer.error = ActivateAbilityResultError::noSuchAbility;

	answer.plan = plan;
	return answer;
}

ValidateAbilityResult NavalBattleEngine::validateExocetData(ExocetData d) const {
    ValidateAbilityResult answer;
    std::set<coord> targets;
    targets.insert(d.target);
    if (d.firingPattern == ExocetData::FiringPattern::plus)
        for (int i = -1; i < 2; i += 2) {//i is -1 and 1
            targets.insert(d.target + coord({ i, 0 }));
            targets.insert(d.target + coord({ 0, i }));
        }
    else if (d.firingPattern == ExocetData::FiringPattern::x)
        for (int i = -1; i < 2; i += 2) //i is -1 and 1
            for (int j = -1; j < 2; j += 2) //j is -1 and 1
                targets.insert(d.target + coord({ i, j }));
    auto r = validateBulkFireTargets(BulkFireData{targets});
    answer.error = r.error;
    answer.plan = ExocetPlan{r.targets};
    return answer;
}

ValidateAbilityResult NavalBattleEngine::validateApacheData(ApacheData d) const{
    ValidateAbilityResult answer;
    std::set<coord> targets;
    targets.insert(d.target);
    if (d.firingPattern == ApacheData::FiringPattern::vertical) {
        targets.insert(d.target + coord({ -1, 0 }));
        targets.insert(d.target + coord({ 1, 0 }));
    }
    else {
        targets.insert(d.target + coord({ 0, -1 }));
        targets.insert(d.target + coord({ 0, 1 }));
    }
    auto r = validateBulkFireTargets(BulkFireData{targets});
    answer.error = r.error;
    answer.plan = ApachePlan{r.targets};
    return answer;
}

ValidateAbilityResult NavalBattleEngine::validateTomahawkData(TomahawkData data) const {
	ValidateAbilityResult answer;
    std::set<coord> targets;
    for (int d = data.target.d - 1; d <= data.target.d + 1; d++)
        for (int o = data.target.o - 1; o <= data.target.o + 1; o++)
            targets.insert(coord({ d,o }));

	auto r = validateBulkFireTargets(BulkFireData{targets});
	answer.error = r.error;
	answer.plan = TomahawkPlan{r.targets};
	return answer;
}

ValidateAbilityResult NavalBattleEngine::validateRelocateData(RelocateData d) const {
	ValidateAbilityResult answer;
	RelocatePlan plan;

	plan.target = d.target;
	plan.shipId = d.shipId;
	plan.willBeOnShip = false;

	if (d.target.d < 1 || d.target.o < 1 || d.target.d >= boardRows() - 1 || d.target.o >= boardCols() - 1)
		answer.error = ActivateAbilityResultError::outOfBounds;

	Player vehicleOwner = getPlayerWithVehicleId(d.shipId);
	const Fleet& fleet = getFleetForPlayer(vehicleOwner);
	const Ship* ship = fleet.getShipById(d.shipId);
	if (ship != nullptr) {
		if (!ship->isPlaced() || ship->isSunk())
			answer.error = ActivateAbilityResultError::shipSunk;
		else if (!ship->hasAbility(VehicleAbilityType::relocate))
			answer.error = ActivateAbilityResultError::noSuchAbility;
	}
	else {
		const Plane* plane = fleet.getPlaneById(d.shipId);
		if(plane != nullptr) {
			if (!plane->isPlaced() || plane->isDestroyed())
				answer.error = ActivateAbilityResultError::shipSunk;
			else if (!plane->hasAbility(VehicleAbilityType::relocate))
				answer.error = ActivateAbilityResultError::noSuchAbility;
		}
		else {
			answer.error = ActivateAbilityResultError::notYourShip;
			return answer;
		}
	}
	answer.plan = plan;
	return answer;
}

ValidateAbilityResult NavalBattle::NavalBattleEngine::validateScanData(ScanData data) const {
    ValidateAbilityResult answer;
	ScanPlan plan;
    std::set<coord> scannedSquares;
    for (int d = data.target.d - 1; d <= data.target.d + 1; d++)
        for (int o = data.target.o - 1; o <= data.target.o + 1; o++) {
			coord c = coord({ d,o });
            if (isValidCoord(c))
                plan.targets.insert(c);
            else
                answer.error = ActivateAbilityResultError::outOfBounds;
        }
	answer.plan = plan;
	return answer;
}

ValidateAbilityResult NavalBattleEngine::validateRevealData(RevealData d, const AbilityContext& ctx) const {
    ValidateAbilityResult answer;
    RevealPlan plan;

    coord vehiclePos = coord::unspecified;
    Player owner = getPlayerWithVehicleId(ctx.vehicleId);
    const Fleet& fleet = getFleetForPlayer(owner);
    if (const Ship* ship = fleet.getShipById(ctx.vehicleId))
        vehiclePos = ship->getPos();
    else if (const Plane* plane = fleet.getPlaneById(ctx.vehicleId)) {
        if (plane->isOnCarrier()) {
            answer.error = ActivateAbilityResultError::noSuchAbility;
            return answer;
        }
        vehiclePos = plane->getPos();
    }

    std::set<coord> squaresToReveal;
    if (d.firingPattern == RevealData::FiringPattern::square) {
        squaresToReveal.insert(coord({ vehiclePos.d - 1, vehiclePos.o - 1 }));
        squaresToReveal.insert(coord({ vehiclePos.d - 1, vehiclePos.o + 1 }));
        squaresToReveal.insert(coord({ vehiclePos.d + 1, vehiclePos.o - 1 }));
        squaresToReveal.insert(coord({ vehiclePos.d + 1, vehiclePos.o + 1 }));
    }
    else if (d.firingPattern == RevealData::FiringPattern::diamond) {
        squaresToReveal.insert(coord({ vehiclePos.d - 1, vehiclePos.o }));
        squaresToReveal.insert(coord({ vehiclePos.d, vehiclePos.o - 1 }));
        squaresToReveal.insert(coord({ vehiclePos.d, vehiclePos.o + 1 }));
        squaresToReveal.insert(coord({ vehiclePos.d + 1, vehiclePos.o }));
    }
    else {
        answer.error = ActivateAbilityResultError::noSuchAbility;
        return answer;
    }
    for (const coord& c : squaresToReveal)
        if (isValidCoord(c))
            plan.targets.insert(c);
        else
            answer.error = ActivateAbilityResultError::outOfBounds;
    answer.plan = plan;
    return answer;
}

//build hitmaps and move to gamplay if other player is ready
ReadyUpResult NavalBattleEngine::readyUp(Player p) {
    ReadyUpResult answer;
    auto status = checkFleetStatus(getFleetForPlayer(p));
    if (status.any()) {
        answer.success = false;

        if (status.test((int)FleetStatusBits::unplaced))
            answer.error = ReadyUpError::fleetNotPlaced;
        else if (status.test((int)FleetStatusBits::overlapping)) 
            answer.error = ReadyUpError::fleetPlacementInvalid;
        else if (status.test((int)FleetStatusBits::outOfBounds)) 
            answer.error = ReadyUpError::fleetPlacementInvalid;

        return answer;
    }
    getDataForPlayer(p).isReady = true;
    if (getDataForPlayer(Player::one).isReady && getDataForPlayer(Player::two).isReady) {
        _phase = Phase::playing;
        _currentPlayer = Player::one;
    }

    answer.success = true;
    return answer;
}

bool NavalBattleEngine::isPlayerReady(Player p) const{
    return getDataForPlayer(p).isReady;
}

// --- Gameplay ---
FireResult NavalBattleEngine::fire(Player p, coord target) {
    FireResult answer;
    
    if (_currentPlayer != p) {
        answer.success = false;
        answer.error = FireError::notYourTurn;
        return answer;
    }

	if (!isValidCoord(target)) {
        answer.success = false;
        answer.error = FireError::outOfBounds;
        return answer;
    }

	Fleet::hitFleetShipsResult r = hitCoord(opponent(p), target);

    answer.success = true;
    if (r.success) {
        answer.isHit = true;
        answer.isSink = r.sunk;
        answer.hitId = r.hitID;
    }
    else
        answer.isHit = false;

    if (answer.success)
        advanceTurn();

    return answer;
}

// --- Queries ---
Phase NavalBattleEngine::phase() const {
    return _phase;
}

Player NavalBattleEngine::getWinner() const{
    if (_phase != Phase::finished)
        return Player::none;
    if (getFleetForPlayer(Player::one).isDefeated())
        return Player::two;
    else
        return Player::one;
}

Player NavalBattleEngine::currentTurn() const {
    return _currentPlayer;
}

std::string NavalBattleEngine::nameForId(int id) const {
    for (const Ship& s : getFleetForPlayer(Player::one).getShips())
        if (s.getId() == id)
            return s.getName();
    for (const Ship& s : getFleetForPlayer(Player::two).getShips())
        if (s.getId() == id)
            return s.getName();
    for (const Plane& p : getFleetForPlayer(Player::one).getPlanes())
        if (p.getId() == id)
            return p.getName();
    for (const Plane& p : getFleetForPlayer(Player::two).getPlanes())
        if (p.getId() == id)
            return p.getName();

    return "";
}

Fleet::hitFleetShipsResult NavalBattleEngine::hitCoord(Player p, coord target) {
    auto& f = getDataForPlayer(p).fleet;
    auto r = f.hitFleetShips(target);
    auto& opponentData = getDataForPlayer(opponent(p));
    if (r.success) {
        clearScansWithSquareForPlayer(opponent(p), target);
        opponentData.hits.insert(target);
        opponentData.revealedHits.erase(target);

        //this belongs here because the game will always end on hitting a coord
        if (f.isDefeated()) {
            _phase = Phase::finished;
            _currentPlayer = Player::none;
        }
    }
    else if(r.error != Fleet::hitFleetShipsError::coordAlreadyHit) 
		opponentData.misses.insert(target);
    
    return r;
}

void NavalBattleEngine::clearScansWithSquareForPlayer(Player p, coord c) {
    std::erase_if(getDataForPlayer(p).scansWithHits, [c](const std::set<coord>& s) {
        return s.contains(c);
        });
}

int NavalBattleEngine::boardRows() const{
    return _boardDimensions.first;
}

int NavalBattleEngine::boardCols() const{
    return _boardDimensions.second;
}

bool NavalBattleEngine::checkCoord(Player p, coord where) {
    return getDataForPlayer(p).fleet.wouldBeHit(where);
}

void NavalBattleEngine::advanceTurn() {
    _currentPlayer = opponent(_currentPlayer);
}

const std::set<coord>& NavalBattleEngine::getHitsForPlayer(Player p) const{
	return getDataForPlayer(p).hits;
}

const std::set<coord>& NavalBattleEngine::getMissesForPlayer(Player p) const{
	return getDataForPlayer(p).misses;
}

bool NavalBattle::NavalBattleEngine::playerHasAntiAircraftGun(Player p) const {
	return getDataForPlayer(p).hasAntiAircraftGun;
}

BoardView NavalBattleEngine::boardViewForPlayer(Player p) const{
    BoardView b;
    b.ownGrid = ownGrid(p);
    b.opponentGrid = opponentGrid(p);
    return b;
}

VehicleView NavalBattleEngine::vehicleViewForPlayer(Player viewer) const {
    VehicleView v;
    v.yourFleet = getViewOfOwnFleet(viewer);
    v.opponentFleet = getViewOfOpponentFleet(viewer);
    return v;
}

GridView NavalBattleEngine::ownGrid(Player p) const {
    GridView occupied;
    const Fleet& f = getFleetForPlayer(p);
    
    for (const auto& c : getDataForPlayer(opponent(p)).revealedHits)
        occupied[c] = SquareState::revealedHit;
    for (const auto& c : getMissesForPlayer(opponent(p)))
        occupied[c] = SquareState::miss;
    for (const auto& c : getHitsForPlayer(opponent(p)))
        occupied[c] = SquareState::hit;
    return occupied;
}

GridView NavalBattleEngine::opponentGrid(Player p) const {
    std::map<coord, SquareState> occupied;

    // Reveal opponent's ships when game is finished
    if (_phase == Phase::finished) {
        const Fleet& oppFleet = getFleetForPlayer(opponent(p));
        for (const Ship& s : oppFleet.getShips())
            if(s.isPlaced())
                for (const coord& c : s.getCoords())
                    occupied[c.applyTransform(s.getPos(), s.getRotation())] = SquareState::ship;
    }

    for (const auto& s : getDataForPlayer(p).scansWithHits)
        for (const auto& c : s)
            occupied[c] = SquareState::scannedPositive;
    for (const auto& c : getDataForPlayer(p).revealedMisses)
        occupied[c] = SquareState::revealedMiss;
    for (const auto& c : getDataForPlayer(p).revealedHits)
        occupied[c] = SquareState::revealedHit;
    for (const auto& c : getMissesForPlayer(p))
        occupied[c] = SquareState::miss;
    for (const auto& c : getHitsForPlayer(p))
        occupied[c] = SquareState::hit;
    return GridView(occupied);
}

Fleet NavalBattleEngine::createFleetFromBlueprint(const FleetBlueprint& blueprint){
    Fleet answer;

    for(const ShipBlueprint& sb : blueprint.ships)
		answer.addShip(Ship(sb, getNextVehicleId()));

	for (const PlaneBlueprint& pb : blueprint.planes)
		answer.addPlane(Plane(pb, getNextVehicleId()));

	return answer;
}

VehicleId NavalBattleEngine::getNextVehicleId() {
    return _nextVehicleId++;
}

NavalBattleEngine::PlayerData NavalBattle::NavalBattleEngine::createPlayerDataForGameMode(GameMode mode) {
	PlayerData data(createFleetFromBlueprint(getBlueprintForMode(mode)));
	if (mode == GameMode::advanced)
		data.hasAntiAircraftGun = true;
	return data;
}

Player NavalBattleEngine::getPlayerWithVehicleId(VehicleId id) const {
	const Fleet& p1Fleet = getFleetForPlayer(Player::one);
    if (p1Fleet.getShipById(id) != nullptr || p1Fleet.getPlaneById(id) != nullptr)
		return Player::one;
    const Fleet& p2Fleet = getFleetForPlayer(Player::two);
	if (p2Fleet.getShipById(id) != nullptr || p2Fleet.getPlaneById(id) != nullptr)
		return Player::two;
	return Player::none;
}

bool NavalBattleEngine::isValidCoord(coord c) const {
	//should coord::unspecified be considered invalid? For now, treat it as valid
    return c.d >= 0 && c.d < boardRows() && c.o >= 0 && c.o < boardCols();
}

bool NavalBattleEngine::isValidCoord(const std::set<coord>& coords) const{
    for (coord c : coords)
        if (!isValidCoord(c))
            return false;
    return true;
}

ActivateAbilityResultError NavalBattle::NavalBattleEngine::playerMayActivateAbility(Player p, int shipId, const VehicleAbilityType& vehicleAbilityType) const {
	if (!playerMayAct(p))
        return ActivateAbilityResultError::notYourTurn;

    Fleet::AbilityAvailabilityError e = getDataForPlayer(p).fleet.abilityAvailable(shipId, vehicleAbilityType);
    if (e != Fleet::AbilityAvailabilityError::none)
        return toActivateAbilityResultError(e);

    return ActivateAbilityResultError::none;
}

FireError NavalBattle::NavalBattleEngine::playerMayFire(Player p, coord target) const {
	if (!playerMayAct(p))
        return FireError::notYourTurn;
	if (!isValidCoord(target))
        return FireError::outOfBounds;
	return FireError::none;
}

bool NavalBattleEngine::playerMayAct(Player p) const {
    if (_currentPlayer != p)
        return false;
    if (_phase != Phase::playing)
        return false;
    return true;
}

std::set<coord>& NavalBattleEngine::getHitsForPlayer(Player p) {
	return getDataForPlayer(p).hits;
}

std::set<coord>& NavalBattleEngine::getMissesForPlayer(Player p) {
	return getDataForPlayer(p).misses;
}

//default fleet for normal game
FleetBlueprint const& NavalBattleEngine::getBaseFleetBlueprint() const{
    static FleetBlueprint baseFleet{
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

//fleet with abilities for advanced game
FleetBlueprint const& NavalBattleEngine::getAdvancedFleetBlueprint() const{
	static PlaneBlueprint reconOneBlueprint = Plane::reconPlane;
	reconOneBlueprint.name = "Recon One";
	static PlaneBlueprint reconTwoBlueprint = Plane::reconPlane;
	reconTwoBlueprint.name = "Recon Two";

    static FleetBlueprint advancedFleet{
        {
            Ship::advancedCarrier,
            Ship::advancedBattleship,
            Ship::advancedDestroyer,
            Ship::advancedSub,
            Ship::pt,
        },
        {
            reconOneBlueprint,
            reconTwoBlueprint
        }
    };
    return advancedFleet;
}

FleetBlueprint const& NavalBattleEngine::getBlueprintForMode(GameMode mode) const{
    if (mode == GameMode::advanced)
        return getAdvancedFleetBlueprint();
    return getBaseFleetBlueprint();
}

std::pair<int, int> NavalBattleEngine::getBoardDimensionsForMode(GameMode mode) {
    switch (mode) {
        case GameMode::advanced: return {10, 14};
        case GameMode::classic:  return {10, 10};
    }
    return {10, 10};
}

std::bitset<8> NavalBattleEngine::checkFleetStatus(const Fleet& f) {
    std::bitset<8> answer;
    std::unordered_set<coord> occupied;
    std::unordered_set<coord> planeAllowedCoords; //squares that planes are allowed to occupy

    //check ships
    for (const Ship& s : f.getShips()) {
        if (s.isPlaced()) { //only check ships that are placed
            for (coord c : s.getCoords()) {
                coord transformed = c.applyTransform(s.getPos(), s.getRotation());
				if (!isValidCoord(transformed)){
                     answer.set((int)FleetStatusBits::outOfBounds, true);
					 continue;//cannot be out of bounds and overlapping
				}
                size_t occupiedSize = occupied.size();
                occupied.insert(transformed);
                if (occupiedSize == occupied.size())
                    answer.set((int)FleetStatusBits::overlapping, true);
                if(s.canHoldPlanes())
    				planeAllowedCoords.insert(transformed);
            }
        }
        else {
            answer.set((int)FleetStatusBits::unplaced, true);
        }
    }

	std::set<coord> planeOccupied;
	//check planes
	for (const Plane& p : f.getPlanes()) {
		if (p.isPlaced()) {
			coord pos = p.getPos();
			if (!planeAllowedCoords.contains(pos)) {
				answer.set((int)FleetStatusBits::outOfBounds, true);
				continue;
			}
			if (planeOccupied.find(pos) != planeOccupied.end())
				answer.set((int)FleetStatusBits::overlapping, true);
			planeOccupied.insert(pos);
		}
        else
        {
			answer.set((int)FleetStatusBits::unplaced, true);
        }
	}

    return answer;
}

NavalBattleEngine::PlayerData& NavalBattleEngine::getDataForPlayer(Player p) {
    if (p == Player::one)
        return _p1Data;
    else if (p == Player::two)
        return _p2Data;

    return _pNoneData;
}

const NavalBattleEngine::PlayerData& NavalBattleEngine::getDataForPlayer(Player p) const {
    return const_cast<NavalBattleEngine*>(this)->getDataForPlayer(p);
}

ActivateAbilityResultError NavalBattle::NavalBattleEngine::toActivateAbilityResultError(Fleet::AbilityAvailabilityError e) const {
    switch (e) {
    case Fleet::AbilityAvailabilityError::none:
        return ActivateAbilityResultError::none;
    case Fleet::AbilityAvailabilityError::invalidID:
        return ActivateAbilityResultError::notYourShip;
    case Fleet::AbilityAvailabilityError::vehicleHasNoSuchAbility:
        return ActivateAbilityResultError::noSuchAbility;
    case Fleet::AbilityAvailabilityError::vehicleDestroyed:
        return ActivateAbilityResultError::shipSunk;
    }
	return ActivateAbilityResultError::noSuchAbility; //default case should never be hit
}
