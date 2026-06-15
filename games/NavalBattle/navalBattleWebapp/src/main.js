import "./style.css";

// === DOM References ===
const userIdInput = document.getElementById("userid");
const gameIdInput = document.getElementById("gameid");
const connectBtn = document.getElementById("connectBtn");
const statusSpan = document.getElementById("status");
const statusBadge = document.getElementById("statusBadge");
const logPre = document.getElementById("log");

const connectSection = document.getElementById("connect");
const gameSection = document.getElementById("game");

const meSpan = document.getElementById("me");
const opponentSpan = document.getElementById("opponent");
const phaseSpan = document.getElementById("phase");
const turnIndicator = document.getElementById("turnIndicator");
const youReadyBadge = document.getElementById("youReadyBadge");
const opponentReadyBadge = document.getElementById("opponentReadyBadge");

const setupControls = document.getElementById("setupControls");
const rotateBtn = document.getElementById("rotateBtn");
const readyBtn = document.getElementById("readyBtn");

const messageToast = document.getElementById("messageToast");
const messageText = document.getElementById("messageText");

const gameOverOverlay = document.getElementById("gameOverOverlay");
const gameOverTitle = document.getElementById("gameOverTitle");
const gameOverMessage = document.getElementById("gameOverMessage");
const rematchBtn = document.getElementById("rematchBtn");
const homeBtn = document.getElementById("homeBtn");

const ownGrid = document.getElementById("ownGrid");
const oppGrid = document.getElementById("oppGrid");
const ownColLabels = document.getElementById("ownColLabels");
const ownRowLabels = document.getElementById("ownRowLabels");
const oppColLabels = document.getElementById("oppColLabels");
const oppRowLabels = document.getElementById("oppRowLabels");

const yourFleetList = document.getElementById("yourFleetList");
const opponentFleetList = document.getElementById("opponentFleetList");

// === State ===
let socket = null;
let rotation = 0;
let lastSetupInfo = null;
let messageTimeout = null;
let lastPhase = null;
let placedShipIds = new Set();
let placedPlaneIds = new Set();
let hoveredCell = null; // Track currently hovered cell for preview refresh
let myUserId = null;
let gameId = null;
let rematchRequested = false;
let opponentWantsRematch = false;
let selectedVehicle = null; // { type: 'ship' | 'plane', id: number }
let activeAbility = null; // { vehicleId: number, type: string, firingPattern: string | null }
let antiAircraftMode = false; // true when player is selecting a target for anti-aircraft fire

const CONNECTION_TOKEN_STORAGE_PREFIX = "navalbattle.connectiontoken.";

function connectionTokenStorageKey(userId, gameId) {
    return `${CONNECTION_TOKEN_STORAGE_PREFIX}${userId}::${gameId}`;
}

function getStoredConnectionToken(userId, gameId) {
    if (!userId || !gameId) return "";
    return localStorage.getItem(connectionTokenStorageKey(userId, gameId)) || "";
}

function storeConnectionToken(userId, gameId, token) {
    if (!userId || !gameId || !token) return;
    localStorage.setItem(connectionTokenStorageKey(userId, gameId), token);
}

// Ability configuration - what additional options each ability needs
const ABILITY_CONFIG = {
    torpedo:  { needsTarget: true, needsFiringPattern: true,  patterns: ["vertical", "horizontal"], targetType: "opponent" },
    exocet:   { needsTarget: true, needsFiringPattern: true,  patterns: ["plus", "x"],              targetType: "opponent" },
    apache:   { needsTarget: true, needsFiringPattern: true,  patterns: ["vertical", "horizontal"], targetType: "opponent" },
    tomahawk: { needsTarget: true, needsFiringPattern: false,                                        targetType: "opponent" },
    scan:     { needsTarget: true, needsFiringPattern: false,                                        targetType: "opponent" },
    reveal:   { needsTarget: false, needsFiringPattern: true, patterns: ["square", "diamond"] },
    relocate: { needsTarget: true, needsFiringPattern: false,                                        targetType: "dynamic", needsShipId: true }
};

// === Utility Functions ===
function logLine(text) {
    const timestamp = new Date().toLocaleTimeString();
    logPre.textContent += `[${timestamp}] ${text}\n`;
    logPre.scrollTop = logPre.scrollHeight;
}

function showMessage(text, type = "info") {
    messageText.textContent = text;
    messageToast.className = `message-toast ${type}`;
    messageToast.classList.remove("hidden");
    
    if (messageTimeout) clearTimeout(messageTimeout);
    messageTimeout = setTimeout(() => {
        messageToast.classList.add("hidden");
    }, 3500);
}

function setConnectionStatus(status) {
    statusSpan.textContent = status.charAt(0).toUpperCase() + status.slice(1);
    statusBadge.className = `status-badge ${status}`;
}

function updatePhaseDisplay(phase) {
    phaseSpan.textContent = phase;
    phaseSpan.className = `phase-badge ${phase}`;

    // Show/hide setup controls based on phase
    if (phase === "setup") {
        setupControls.classList.remove("hidden");
        gameSection.classList.add("phase-setup");
    } else {
        setupControls.classList.add("hidden");
        gameSection.classList.remove("phase-setup");
    }
}

function updateTurnIndicator(currentTurn, myUserId) {
    if (!currentTurn) {
        turnIndicator.classList.add("hidden");
        return;
    }
    
    turnIndicator.classList.remove("hidden");
    
    if (currentTurn === myUserId) {
        turnIndicator.textContent = "Your Turn";
        turnIndicator.className = "turn-indicator";
    } else {
        turnIndicator.textContent = "Opponent's Turn";
        turnIndicator.className = "turn-indicator not-your-turn";
    }
}

function updateReadyStatus(youReady, opponentReady) {
    youReadyBadge.classList.toggle("hidden", !youReady);
    opponentReadyBadge.classList.toggle("hidden", !opponentReady);
    readyBtn.classList.toggle("is-ready", youReady);
}

function showGameOver(isVictory) {
    gameOverOverlay.classList.remove("hidden");
    const content = gameOverOverlay.querySelector(".game-over-content");
    
    // Reset rematch state
    rematchRequested = false;
    opponentWantsRematch = false;
    updateRematchButton();
    
    if (isVictory) {
        content.classList.add("victory");
        content.classList.remove("defeat");
        gameOverTitle.textContent = "Victory!";
        gameOverMessage.textContent = "You sank all enemy ships!";
    } else {
        content.classList.add("defeat");
        content.classList.remove("victory");
        gameOverTitle.textContent = "Defeat";
        gameOverMessage.textContent = "Your fleet has been destroyed.";
    }
}

function updateRematchButton() {
    if (rematchRequested && opponentWantsRematch) {
        rematchBtn.textContent = "Starting Rematch...";
        rematchBtn.disabled = true;
    } else if (rematchRequested) {
        rematchBtn.textContent = "Waiting for opponent...";
        rematchBtn.disabled = true;
    } else if (opponentWantsRematch) {
        rematchBtn.textContent = "Accept Rematch";
        rematchBtn.disabled = false;
    } else {
        rematchBtn.textContent = "Rematch";
        rematchBtn.disabled = false;
    }
}

function hideGameOver() {
    gameOverOverlay.classList.add("hidden");
}

// === Grid Building ===
function buildGridLabels(colContainer, rowContainer, rows, cols) {
    colContainer.innerHTML = "";
    rowContainer.innerHTML = "";
    
    // Column labels: 1, 2, 3, ...
    for (let c = 0; c < cols; c++) {
        const label = document.createElement("span");
        label.textContent = String(c + 1);
        colContainer.appendChild(label);
    }
    
    // Row labels: A, B, C, ...
    for (let r = 0; r < rows; r++) {
        const label = document.createElement("span");
        label.textContent = String.fromCharCode(65 + r); // A=65
        rowContainer.appendChild(label);
    }
}

function buildGrids(rows, cols) {
    ownGrid.style.setProperty("--rows", rows);
    ownGrid.style.setProperty("--cols", cols);
    oppGrid.style.setProperty("--rows", rows);
    oppGrid.style.setProperty("--cols", cols);

    // Build labels
    buildGridLabels(ownColLabels, ownRowLabels, rows, cols);
    buildGridLabels(oppColLabels, oppRowLabels, rows, cols);

    ownGrid.innerHTML = "";
    oppGrid.innerHTML = "";

    // Build own grid
    for (let r = 0; r < rows; r++) {
        for (let c = 0; c < cols; c++) {
            const cell = document.createElement("div");
            cell.className = "cell";
            cell.dataset.row = r;
            cell.dataset.col = c;

            cell.addEventListener("click", () => handleOwnGridClick(r, c));
            cell.addEventListener("mouseenter", () => handleOwnGridHover(r, c));
            cell.addEventListener("mouseleave", () => clearPreviewAndHover());
            ownGrid.appendChild(cell);
        }
    }

    // Build opponent grid
    for (let r = 0; r < rows; r++) {
        for (let c = 0; c < cols; c++) {
            const cell = document.createElement("div");
            cell.className = "cell";
            cell.dataset.row = r;
            cell.dataset.col = c;

            cell.addEventListener("click", () => handleOppGridClick(r, c));
            cell.addEventListener("mouseenter", () => handleOppGridHover(r, c));
            cell.addEventListener("mouseleave", () => clearAbilityPreview());
            oppGrid.appendChild(cell);
        }
    }
}

// === Fleet Panel Rendering ===
function getAbilityIcon(abilityType) {
    const icons = {
        torpedo: "\u{1F4A5}",      // ?? explosion
        exocet: "\u{1F680}",       // ?? rocket
        apache: "\u{1F681}",       // ?? helicopter
        tomahawk: "\u2604\uFE0F",  // ?? comet/meteor
        scan: "\u{1F50D}",         // ?? magnifying glass
        reveal: "\u{1F441}\uFE0F", // ??? eye
        relocate: "\u21C4"         // ? arrows
    };
    return icons[abilityType] || "\u2753";
}

function getAbilityDisplayName(abilityType) {
    const names = {
        torpedo: "Torpedo",
        exocet: "Exocet",
        apache: "Apache",
        tomahawk: "Tomahawk",
        scan: "Scan",
        reveal: "Reveal",
        relocate: "Relocate"
    };
    return names[abilityType] || abilityType;
}

function calculateShipBounds(coords) {
    if (!coords || coords.length === 0) return { minRow: 0, maxRow: 0, minCol: 0, maxCol: 0, rows: 1, cols: 1 };

    let minRow = Infinity, maxRow = -Infinity;
    let minCol = Infinity, maxCol = -Infinity;

    for (const c of coords) {
        if (c.row < minRow) minRow = c.row;
        if (c.row > maxRow) maxRow = c.row;
        if (c.col < minCol) minCol = c.col;
        if (c.col > maxCol) maxCol = c.col;
    }

    return {
        minRow,
        maxRow,
        minCol,
        maxCol,
        rows: maxRow - minRow + 1,
        cols: maxCol - minCol + 1
    };
}

function createShipFormGrid(coords) {
    const bounds = calculateShipBounds(coords);
    const container = document.createElement("div");
    container.className = "ship-form";

    const grid = document.createElement("div");
    grid.className = "ship-form-grid";
    grid.style.gridTemplateColumns = `repeat(${bounds.cols}, 12px)`;
    grid.style.gridTemplateRows = `repeat(${bounds.rows}, 12px)`;

    // Create a set for quick lookup
    const coordSet = new Set(coords.map(c => `${c.row},${c.col}`));

    // Fill grid
    for (let r = bounds.minRow; r <= bounds.maxRow; r++) {
        for (let c = bounds.minCol; c <= bounds.maxCol; c++) {
            const cell = document.createElement("div");
            cell.className = "ship-form-cell";
            if (coordSet.has(`${r},${c}`)) {
                cell.classList.add("ship");
            } else {
                cell.classList.add("empty");
            }
            grid.appendChild(cell);
        }
    }

    container.appendChild(grid);
    return container;
}

function createAbilityButton(ability, shipId, isYours, vehicleDestroyed = false) {
    const element = document.createElement(isYours ? "button" : "div");
    element.className = isYours ? "ability-btn" : "ability-display";
    element.dataset.abilityType = ability.type;
    element.dataset.vehicleId = shipId;

    const icon = document.createElement("span");
    icon.className = "ability-icon";
    icon.textContent = getAbilityIcon(ability.type);

    const name = document.createElement("span");
    name.className = "ability-name";
    name.textContent = getAbilityDisplayName(ability.type);

    const uses = document.createElement("span");
    uses.className = "ability-uses";
    if (ability.usagepolicy === "unlimited") {
        uses.textContent = "\u221E"; // ? infinity symbol
        uses.classList.add("unlimited");
    } else {
        uses.textContent = `${ability.remaininguses}`;
    }

    element.appendChild(icon);
    element.appendChild(name);
    element.appendChild(uses);

    if (isYours) {
        element.disabled = !ability.canuse || lastPhase === "setup" || vehicleDestroyed;
        element.addEventListener("click", () => handleAbilityClick(shipId, ability.type));
    }

    return element;
}

function createShipCard(ship, isYours) {
    const card = document.createElement("div");
    card.className = "ship-card";
    card.dataset.shipId = ship.id;

    if (ship.issunk) {
        card.classList.add("sunk");
    }

    // Mark as placed if ship has valid coords
    const isPlaced = ship.coords !== undefined && ship.coords !== null;
    if (isPlaced) {
        card.classList.add("placed");
    }

    // Mark as selected during setup
    if (isYours && selectedVehicle && selectedVehicle.type === 'ship' && selectedVehicle.id === ship.id) {
        card.classList.add("selected");
    }

    // Make clickable during setup (placed ships can be re-selected to reposition)
    if (isYours && lastPhase === "setup") {
        card.classList.add("selectable");
        card.addEventListener("click", () => selectVehicle('ship', ship.id));
    }

    // Ship name
    const nameEl = document.createElement("div");
    nameEl.className = "ship-name";
    nameEl.textContent = ship.name;
    card.appendChild(nameEl);

    // Ship form (mini grid visualization)
    const formGrid = createShipFormGrid(ship.shape);
    card.appendChild(formGrid);

    // Abilities
    if (ship.abilities && ship.abilities.length > 0) {
        const abilitiesContainer = document.createElement("div");
        abilitiesContainer.className = "ship-abilities";

        for (const ability of ship.abilities) {
            const abilityEl = createAbilityButton(ability, ship.id, isYours, ship.issunk);
            abilitiesContainer.appendChild(abilityEl);
        }

        card.appendChild(abilitiesContainer);
    }

    return card;
}

// Plane colors array - matches CSS plane-color-N classes
const PLANE_COLORS = ['#ef4444', '#3b82f6', '#8b5cf6', '#22c55e', '#06b6d4', '#f59e0b'];

function createPlaneCard(plane, isYours, planeIndex = 0) {
    const card = document.createElement("div");
    card.className = "plane-card";
    card.dataset.planeId = plane.id;

    if (plane.isdestroyed) {
        card.classList.add("destroyed");
    }

    // Mark as placed if plane has valid position
    const isPlaced = plane.position !== undefined && plane.position !== null;
    if (isPlaced) {
        card.classList.add("placed");
    }

    // Mark as selected during setup
    if (isYours && selectedVehicle && selectedVehicle.type === 'plane' && selectedVehicle.id === plane.id) {
        card.classList.add("selected");
    }

    // Make clickable during setup (placed planes can be re-selected to reposition)
    if (isYours && lastPhase === "setup") {
        card.classList.add("selectable");
        card.addEventListener("click", () => selectVehicle('plane', plane.id));
    }

    // Plane name
    const nameEl = document.createElement("div");
    nameEl.className = "plane-name";
    nameEl.textContent = plane.name;
    card.appendChild(nameEl);

    // Plane icon/indicator with color
    const planeColor = PLANE_COLORS[planeIndex % PLANE_COLORS.length];
    const iconEl = document.createElement("div");
    iconEl.className = "plane-icon";
    iconEl.textContent = "\u2708";
    iconEl.style.color = planeColor;
    card.appendChild(iconEl);

    // Position indicator (if placed)
    if (isPlaced) {
        const posEl = document.createElement("div");
        posEl.className = "plane-position";
        const rowLabel = String.fromCharCode(65 + plane.position.row);
        posEl.textContent = `${rowLabel}${plane.position.col + 1}`;
        card.appendChild(posEl);
    }

    // Abilities
    if (plane.abilities && plane.abilities.length > 0) {
        const abilitiesContainer = document.createElement("div");
        abilitiesContainer.className = "plane-abilities";

        for (const ability of plane.abilities) {
            const abilityEl = createAbilityButton(ability, plane.id, isYours, plane.isdestroyed);
            abilitiesContainer.appendChild(abilityEl);
        }

        card.appendChild(abilitiesContainer);
    }

    return card;
}

function renderFleetPanel(container, ships, planes, isYours) {
    container.innerHTML = "";

    const hasShips = ships && ships.length > 0;
    const hasPlanes = planes && planes.length > 0;

    if (!hasShips && !hasPlanes) {
        const emptyMsg = document.createElement("div");
        emptyMsg.className = "fleet-empty";
        emptyMsg.textContent = "No vehicles";
        container.appendChild(emptyMsg);
        return;
    }

    // Render ships
    if (hasShips) {
        for (const ship of ships) {
            const card = createShipCard(ship, isYours);
            container.appendChild(card);
        }
    }

    // Render planes with color index
    if (hasPlanes) {
        planes.forEach((plane, planeIndex) => {
            const card = createPlaneCard(plane, isYours, planeIndex);
            container.appendChild(card);
        });
    }

    // Render anti-aircraft button and cancel-ability button below fleet if player has them
    const fleetHasAbilities = isYours && lastPhase !== "setup" &&
        ([...(ships || []), ...(planes || [])].some(v => v.abilities && v.abilities.length > 0));

    // Remove any previously rendered action row from the panel so re-renders don't duplicate it
    const panel = container.closest(".fleet-panel");
    panel?.querySelector(".fleet-action-row")?.remove();

    if (isYours && (lastSetupInfo?.hasantiaircraft || fleetHasAbilities)) {
        const actionRow = document.createElement("div");
        actionRow.className = "fleet-action-row";

        if (lastSetupInfo?.hasantiaircraft) {
            const aaBtn = document.createElement("button");
            aaBtn.className = "aa-gun-btn";
            aaBtn.id = "antiAircraftBtn";
            if (antiAircraftMode) aaBtn.classList.add("active");
            aaBtn.innerHTML = `<span class="aa-icon">\u{1F6E1}\uFE0F</span> Anti-Aircraft`;
            aaBtn.addEventListener("click", () => handleAntiAircraftClick());
            actionRow.appendChild(aaBtn);
        }

        if (fleetHasAbilities) {
            const cancelBtn = document.createElement("button");
            cancelBtn.className = "cancel-ability-btn";
            cancelBtn.id = "cancelAbilityBtn";
            cancelBtn.disabled = !activeAbility;
            if (activeAbility) cancelBtn.classList.add("active");
            cancelBtn.innerHTML = `✕ Cancel Ability`;
            cancelBtn.addEventListener("click", () => {
                cancelAbilityMode();
                showMessage("Ability cancelled", "info");
            });
            actionRow.appendChild(cancelBtn);
        }

        if (actionRow.children.length > 0)
            panel.appendChild(actionRow);
    }
}


function drawVehiclesOnGrid(vehicleView, cols) {
    if (!vehicleView) return;

    // Draw own ships on own grid
    for (const ship of vehicleView.yourfleet?.ships || []) {
        if (!ship.coords) continue;
        for (const coord of ship.coords) {
            const idx = coord.row * cols + coord.col;
            const cell = ownGrid.children[idx];
            if (cell && !cell.classList.contains("hit") && !cell.classList.contains("revealedhit")) cell.classList.add("ship");
        }
    }

    // Draw own planes: on carrier → ownGrid, deployed (in air) → oppGrid
    (vehicleView.yourfleet?.planes || []).forEach((plane, planeIndex) => {
        if (plane.position && !plane.isdestroyed) {
            const targetGrid = plane.isoncarrier ? ownGrid : oppGrid;
            const idx = plane.position.row * cols + plane.position.col;
            const cell = targetGrid.children[idx];
            if (cell) {
                cell.classList.add("plane");
                cell.classList.add(`plane-color-${planeIndex % PLANE_COLORS.length}`);
            }
        }
    });

    // Draw opponent ships on opponent grid (only if position is known, e.g. sunk ships)
    for (const ship of vehicleView.opponentfleet?.ships || []) {
        const absCoords = getAbsoluteShipCoords(ship);
        if (!absCoords) continue;
        for (const coord of absCoords) {
            const idx = coord.row * cols + coord.col;
            const cell = oppGrid.children[idx];
            if (cell && !cell.classList.contains("hit") && !cell.classList.contains("revealedhit")) cell.classList.add("ship");
        }
    }

    // Draw opponent planes: on carrier → oppGrid, deployed (attacking us) → ownGrid
    (vehicleView.opponentfleet?.planes || []).forEach((plane, planeIndex) => {
        if (plane.position && !plane.isdestroyed) {
            const targetGrid = plane.isoncarrier ? oppGrid : ownGrid;
            const idx = plane.position.row * cols + plane.position.col;
            const cell = targetGrid.children[idx];
            if (cell) {
                cell.classList.add("plane");
                cell.classList.add(`plane-color-${planeIndex % PLANE_COLORS.length}`);
            }
        }
    });
}

function updateFleetPanels(vehicleView) {
    if (!vehicleView) return;

    // Keep lastSetupInfo.vehicleview in sync for re-renders
    if (lastSetupInfo) {
        lastSetupInfo.vehicleview = vehicleView;
    }

    // Render your fleet (ships and planes)
    renderFleetPanel(
        yourFleetList,
        vehicleView.yourfleet?.ships || [],
        vehicleView.yourfleet?.planes || [],
        true
    );

    // Render opponent fleet (ships and planes)
    renderFleetPanel(
        opponentFleetList,
        vehicleView.opponentfleet?.ships || [],
        vehicleView.opponentfleet?.planes || [],
        false
    );
}

function handleAbilityClick(vehicleId, abilityType) {
    if (lastPhase === "setup") return;

    // Toggle off if this ability is already active
    if (activeAbility && activeAbility.vehicleId === vehicleId && activeAbility.type === abilityType) {
        cancelAbilityMode();
        showMessage("Ability cancelled", "info");
        return;
    }

    const config = ABILITY_CONFIG[abilityType];
    if (!config) {
        showMessage(`Unknown ability type: ${abilityType}`, "error");
        return;
    }

    // If ability needs a firing pattern, prompt user to select one first
    if (config.needsFiringPattern) {
        const onPatternSelected = config.needsTarget
            ? (pattern) => activateAbilityMode(vehicleId, abilityType, pattern)
            : (pattern) => executeImmediateAbility(vehicleId, abilityType, pattern);
        showFiringPatternSelector(vehicleId, abilityType, config.patterns, onPatternSelected);
        return;
    }

    // Otherwise, activate ability mode directly
    activateAbilityMode(vehicleId, abilityType, null);
}

function showFiringPatternSelector(vehicleId, abilityType, patterns, onPatternSelected) {
    // Create a simple pattern selector UI
    const selector = document.createElement("div");
    selector.className = "pattern-selector-overlay";
    selector.id = "patternSelector";

    const content = document.createElement("div");
    content.className = "pattern-selector-content";

    const title = document.createElement("h3");
    title.textContent = `Select ${getAbilityDisplayName(abilityType)} Pattern`;
    content.appendChild(title);

    const buttonsDiv = document.createElement("div");
    buttonsDiv.className = "pattern-buttons";

    for (const pattern of patterns) {
        const btn = document.createElement("button");
        btn.className = "btn-secondary pattern-btn";
        btn.textContent = pattern.charAt(0).toUpperCase() + pattern.slice(1);
        btn.addEventListener("click", () => {
            document.getElementById("patternSelector")?.remove();
            onPatternSelected(pattern);
        });
        buttonsDiv.appendChild(btn);
    }

    const cancelBtn = document.createElement("button");
    cancelBtn.className = "btn-secondary";
    cancelBtn.textContent = "Cancel";
    cancelBtn.addEventListener("click", () => {
        document.getElementById("patternSelector")?.remove();
    });
    buttonsDiv.appendChild(cancelBtn);

    content.appendChild(buttonsDiv);
    selector.appendChild(content);
    document.body.appendChild(selector);
}

function isVehicleAPlane(vehicleId) {
    if (!lastSetupInfo || !lastSetupInfo.vehicleview) return false;

    for (const plane of lastSetupInfo.vehicleview.yourfleet?.planes || []) {
        if (plane.id === vehicleId) return true;
    }
    for (const plane of lastSetupInfo.vehicleview.opponentfleet?.planes || []) {
        if (plane.id === vehicleId) return true;
    }

    return false;
}

function executeImmediateAbility(vehicleId, abilityType, firingPattern) {
    if (!lastSetupInfo) return;

    const message = {
        gameid: lastSetupInfo.gameid,
        userid: lastSetupInfo.you,
        sessionaction: {
            type: "activateability",
            data: {
                vehicleid: vehicleId,
                abilityaction: {
                    type: abilityType,
                    data: { firingpattern: firingPattern }
                }
            }
        }
    };

    logLine(`Sending activateability: ${abilityType} (${firingPattern})`);
    socket.send(JSON.stringify(message));
}

function activateAbilityMode(vehicleId, abilityType, firingPattern) {
    activeAbility = { vehicleId, type: abilityType, firingPattern };
    selectedVehicle = null; // Clear placement selection
    cancelAntiAircraftMode(); // Cancel anti-aircraft mode if active

    const config = ABILITY_CONFIG[abilityType];
    let targetGrid;

    // For relocate, determine target grid based on vehicle type
    if (config.targetType === "dynamic") {
        const isPlane = isVehicleAPlane(vehicleId);
        if (isPlane) {
            // Planes relocate to opponent grid only
            targetGrid = "opponent's grid";
            activeAbility.targetType = "opponent";
        } else {
            // Ships relocate to own grid only
            targetGrid = "your grid";
            activeAbility.targetType = "own";
        }
    } else {
        targetGrid = config.targetType === "opponent" ? "opponent's grid" : "your grid";
        activeAbility.targetType = config.targetType;
    }

    showMessage(`${getAbilityDisplayName(abilityType)} active - Click on ${targetGrid} to target`, "info");

    // Add visual indicator to the grids
    updateAbilityModeUI();
}

function cancelAbilityMode() {
    activeAbility = null;
    updateAbilityModeUI();
}

function handleAntiAircraftClick() {
    // Toggle anti-aircraft mode
    if (antiAircraftMode) {
        cancelAntiAircraftMode();
        return;
    }

    // Cancel any active ability first
    cancelAbilityMode();

    antiAircraftMode = true;
    showMessage("Anti-Aircraft active - Click on your grid to target enemy planes", "info");
    updateAntiAircraftModeUI();

    // Re-render fleet panels to show active state
    if (lastSetupInfo && lastSetupInfo.vehicleview) {
        updateFleetPanels(lastSetupInfo.vehicleview);
    }
}

function cancelAntiAircraftMode() {
    antiAircraftMode = false;
    updateAntiAircraftModeUI();

    // Re-render fleet panels to clear active state
    if (lastSetupInfo && lastSetupInfo.vehicleview) {
        updateFleetPanels(lastSetupInfo.vehicleview);
    }
}

function updateAntiAircraftModeUI() {
    const ownGridWrapper = ownGrid.closest(".grid-wrapper");

    if (antiAircraftMode) {
        ownGridWrapper?.classList.add("ability-target");
    } else {
        ownGridWrapper?.classList.remove("ability-target");
    }
}

function executeAntiAircraft(row, col) {
    if (!lastSetupInfo) return;

    const message = {
        gameid: lastSetupInfo.gameid,
        userid: lastSetupInfo.you,
        sessionaction: {
            type: "fireantiaircraft",
            data: {
                target: { row, col },
            },
        },
    };

    logLine(`Sending fireantiaircraft at (${row}, ${col})`);
    sendMessage(message);

    cancelAntiAircraftMode();
}

function updateAbilityModeUI() {
    // Add/remove ability-mode class from grids
    const ownGridWrapper = ownGrid.closest(".grid-wrapper");
    const oppGridWrapper = oppGrid.closest(".grid-wrapper");

    if (activeAbility) {
        const targetType = activeAbility.targetType;
        if (targetType === "opponent") {
            oppGridWrapper?.classList.add("ability-target");
            ownGridWrapper?.classList.remove("ability-target");
        } else if (targetType === "own") {
            ownGridWrapper?.classList.add("ability-target");
            oppGridWrapper?.classList.remove("ability-target");
        }
    } else {
        ownGridWrapper?.classList.remove("ability-target");
        oppGridWrapper?.classList.remove("ability-target");
    }

    // Update cancel button state (lives inside the fleet panel)
    const cancelBtn = document.getElementById("cancelAbilityBtn");
    if (cancelBtn) {
        cancelBtn.disabled = !activeAbility;
        cancelBtn.classList.toggle("active", !!activeAbility);
    }

    // Reflect active state on ability buttons so the user can see which is selected
    document.querySelectorAll(".ability-btn").forEach(btn => {
        const isActive = activeAbility &&
            parseInt(btn.dataset.vehicleId) === activeAbility.vehicleId &&
            btn.dataset.abilityType === activeAbility.type;
        btn.classList.toggle("active", !!isActive);
    });
}

function executeAbility(row, col) {
    if (!activeAbility || !lastSetupInfo) return;

    const { vehicleId, type, firingPattern } = activeAbility;
    const config = ABILITY_CONFIG[type];

    let abilityData;
    switch (type) {
        case "torpedo":
            abilityData = {
                firingpattern: firingPattern,
                startpoint: { row, col }
            };
            break;
        case "exocet":
            abilityData = {
                firingpattern: firingPattern,
                target: { row, col }
            };
            break;
        case "apache":
            abilityData = {
                firingpattern: firingPattern,
                target: { row, col }
            };
            break;
        case "tomahawk":
            abilityData = { target: { row, col } };
            break;
        case "scan":
            abilityData = { target: { row, col } };
            break;
        case "relocate":
            // For relocate, we need to specify which ship to move
            // For now, we'll need another selection step - this is complex
            // Simplified: prompt for ship selection or use the activating vehicle's ship
            abilityData = {
                shipid: vehicleId, // Move the ship that has the ability
                target: { row, col }
            };
            break;
        default:
            showMessage(`Unknown ability: ${type}`, "error");
            cancelAbilityMode();
            return;
    }

    const message = {
        gameid: lastSetupInfo.gameid,
        userid: lastSetupInfo.you,
        sessionaction: {
            type: "activateability",
            data: {
                vehicleid: vehicleId,
                abilityaction: {
                    type: type,
                    data: abilityData
                }
            }
        }
    };

    logLine(`Sending activateability: ${type} at (${row}, ${col})`);
    socket.send(JSON.stringify(message));

    // Clear ability mode after execution
    cancelAbilityMode();
}

function selectVehicle(type, id) {
    selectedVehicle = { type, id };
    // If re-selecting an already-placed vehicle, remove it from the placed
    // sets so auto-advance won't skip it when searching for unplaced vehicles
    if (type === 'ship') placedShipIds.delete(id);
    else if (type === 'plane') placedPlaneIds.delete(id);
    // Re-render fleet panels to show selection
    if (lastSetupInfo && lastSetupInfo.vehicleview) {
        updateFleetPanels(lastSetupInfo.vehicleview);
    }
    showMessage(`Selected ${type} for placement`, "info");
}

function selectNextUnplacedVehicle(vehicleView) {
    for (const ship of vehicleView.yourfleet?.ships || []) {
        const isPlaced = ship.coords !== undefined && ship.coords !== null;
        if (!isPlaced && !placedShipIds.has(ship.id)) {
            selectVehicle('ship', ship.id);
            return;
        }
    }
    for (const plane of vehicleView.yourfleet?.planes || []) {
        const isPlaced = plane.position !== undefined && plane.position !== null;
        if (!isPlaced && !placedPlaneIds.has(plane.id)) {
            selectVehicle('plane', plane.id);
            return;
        }
    }
    selectedVehicle = null;
}

// === Click Handlers ===
function handleOwnGridClick(row, col) {
    // Check if we're in anti-aircraft mode
    if (antiAircraftMode) {
        executeAntiAircraft(row, col);
        return;
    }

    // Check if we're in ability mode targeting own grid
    if (activeAbility) {
        const targetType = activeAbility.targetType;
        if (targetType === "own") {
            executeAbility(row, col);
            return;
        }
    }

    // Ship/plane placement during setup
    if (!selectedVehicle || !lastSetupInfo) return;

    if (selectedVehicle.type === 'ship') {
        const message = {
            gameid: lastSetupInfo.gameid,
            userid: lastSetupInfo.you,
            sessionaction: {
                type: "placeship",
                data: {
                    position: { row, col },
                    rotation: rotation,
                    shipid: selectedVehicle.id,
                },
            },
        };
        sendMessage(message);
    } else if (selectedVehicle.type === 'plane') {
        const message = {
            gameid: lastSetupInfo.gameid,
            userid: lastSetupInfo.you,
            sessionaction: {
                type: "placeplane",
                data: {
                    position: { row, col },
                    planeid: selectedVehicle.id,
                },
            },
        };
        sendMessage(message);
    }
}

function handleOwnGridHover(row, col) {
    hoveredCell = { row, col };

    // Check if we're in ability mode targeting own grid
    if (activeAbility && lastSetupInfo) {
        const targetType = activeAbility.targetType;
        if (targetType === "own") {
            const { vehicleId, type } = activeAbility;

            // For relocate on own grid, show preview
            if (type === "relocate") {
                const abilityData = {
                    shipid: vehicleId,
                    target: { row, col }
                };

                const message = {
                    gameid: lastSetupInfo.gameid,
                    userid: lastSetupInfo.you,
                    sessionaction: {
                        type: "checkability",
                        data: {
                            vehicleid: vehicleId,
                            abilitydata: {
                                type: type,
                                data: abilityData
                            }
                        }
                    }
                };

                sendMessage(message);
            }
            return;
        }
    }

    // Only show preview during setup phase
    if (!selectedVehicle || !lastSetupInfo) return;
    if (lastPhase !== "setup") return;

    if (selectedVehicle.type === 'ship') {
        const message = {
            gameid: lastSetupInfo.gameid,
            userid: lastSetupInfo.you,
            sessionaction: {
                type: "checkplacement",
                data: {
                    position: { row, col },
                    rotation: rotation,
                    shipid: selectedVehicle.id,
                },
            },
        };
        sendMessage(message);
    } else if (selectedVehicle.type === 'plane') {
        const message = {
            gameid: lastSetupInfo.gameid,
            userid: lastSetupInfo.you,
            sessionaction: {
                type: "checkplaneplacement",
                data: {
                    position: { row, col },
                    planeid: selectedVehicle.id,
                },
            },
        };
        sendMessage(message);
    }
}

function clearPreview() {
    // Clear preview overlay classes from all cells in both grids
    const previewClasses = [
        "preview-valid", "preview-invalid", "preview-targeted",
        "torpedoup", "torpedodown", "torpedoleft", "torpedoright"
    ];

    for (const cell of ownGrid.children) {
        cell.classList.remove(...previewClasses);
    }
    for (const cell of oppGrid.children) {
        cell.classList.remove(...previewClasses);
    }
}

function clearAbilityPreview() {
    // Clear ability previews from opponent grid
    const previewClasses = [
        "preview-valid", "preview-invalid", "preview-targeted",
        "torpedoup", "torpedodown", "torpedoleft", "torpedoright"
    ];

    for (const cell of oppGrid.children) {
        cell.classList.remove(...previewClasses);
    }
}

function clearPreviewAndHover() {
    hoveredCell = null;
    clearPreview();
}

function handleOppGridHover(row, col) {
    // Only show preview if we're in ability mode targeting opponent grid
    if (!activeAbility || !lastSetupInfo) return;

    const targetType = activeAbility.targetType;
    if (targetType !== "opponent") return;

    const { vehicleId, type, firingPattern } = activeAbility;

    // Construct ability data based on type
    let abilityData;
    switch (type) {
        case "torpedo":
            abilityData = {
                firingpattern: firingPattern,
                startpoint: { row, col }
            };
            break;
        case "exocet":
            abilityData = {
                firingpattern: firingPattern,
                target: { row, col }
            };
            break;
        case "apache":
            abilityData = {
                firingpattern: firingPattern,
                target: { row, col }
            };
            break;
        case "tomahawk":
            abilityData = { target: { row, col } };
            break;
        case "scan":
            abilityData = { target: { row, col } };
            break;
        case "relocate":
            abilityData = {
                shipid: vehicleId,
                target: { row, col }
            };
            break;
        default:
            return;
    }

    const message = {
        gameid: lastSetupInfo.gameid,
        userid: lastSetupInfo.you,
        sessionaction: {
            type: "checkability",
            data: {
                vehicleid: vehicleId,
                abilitydata: {
                    type: type,
                    data: abilityData
                }
            }
        }
    };

    sendMessage(message);
}

function handleOppGridClick(row, col) {
    if (!lastSetupInfo) return;

    // Ignore opponent grid clicks while anti-aircraft mode is active
    if (antiAircraftMode) return;

    // Check if we're in ability mode targeting opponent grid
    if (activeAbility) {
        const targetType = activeAbility.targetType;
        if (targetType === "opponent") {
            executeAbility(row, col);
            return;
        }
    }

    // Default: regular fire action
    const message = {
        gameid: lastSetupInfo.gameid,
        userid: lastSetupInfo.you,
        sessionaction: {
            type: "fire",
            data: {
                target: { row, col },
            },
        },
    };

    sendMessage(message);
}

function sendMessage(msg) {
    if (socket && socket.readyState === WebSocket.OPEN) {
        socket.send(JSON.stringify(msg));
        logLine("Sent: " + JSON.stringify(msg));
    }
}

// === Event Handlers ===
rotateBtn.addEventListener("click", () => {
    rotation = (rotation + 1) % 4;
    const directions = ["Right", "Up", "Left", "Down"];
    showMessage(`Rotation: ${directions[rotation]}`, "info");
    
    // Refresh preview if hovering over a cell
    if (hoveredCell) {
        handleOwnGridHover(hoveredCell.row, hoveredCell.col);
    }
});

// Keyboard shortcut for rotation
document.addEventListener("keydown", (e) => {
    if (e.key === "r" || e.key === "R") {
        // Don't trigger if user is typing in an input
        if (e.target.tagName === "INPUT" || e.target.tagName === "TEXTAREA") return;
        if (lastPhase !== "setup") return;

        rotateBtn.click();
    }

    // Cancel ability mode with Escape
    if (e.key === "Escape") {
        if (activeAbility) {
            cancelAbilityMode();
            showMessage("Ability cancelled", "info");
        }
        if (antiAircraftMode) {
            cancelAntiAircraftMode();
            showMessage("Anti-Aircraft cancelled", "info");
        }
        // Also close pattern selector if open
        document.getElementById("patternSelector")?.remove();
    }
});

readyBtn.addEventListener("click", () => {
    if (!lastSetupInfo) return;

    const message = {
        gameid: lastSetupInfo.gameid,
        userid: lastSetupInfo.you,
        sessionaction: {
            type: "ready",
            data: null,
        },
    };

    sendMessage(message);
});

document.getElementById("connectForm").addEventListener("submit", (e) => {
    e.preventDefault();
    connectBtn.click();
});

connectBtn.addEventListener("click", () => {
    if (socket && (socket.readyState === WebSocket.CONNECTING || socket.readyState === WebSocket.OPEN)) {
        showMessage("Already connected. Use the existing session.", "info");
        return;
    }

    const userId = userIdInput.value.trim();
    const gameIdValue = gameIdInput.value.trim();

    if (!userId || !gameIdValue) {
        showMessage("Please enter both User ID and Game ID", "error");
        return;
    }

    // Store globally
    myUserId = userId;
    gameId = gameIdValue;

    setConnectionStatus("connecting");

    const WS_URL = import.meta.env.VITE_WS_URL;
    if (!WS_URL) {
        showMessage("Missing VITE_WS_URL configuration", "error");
        throw new Error("Missing VITE_WS_URL. Create webapp/.env.local (see .env.example).");
    }

    socket = new WebSocket(WS_URL);

    socket.onopen = () => {
        setConnectionStatus("connected");
        logLine("WebSocket connected");

        const helloMessage = {
            type: "hello",
            userid: myUserId,
            gameid: gameId,
            connectiontoken: getStoredConnectionToken(myUserId, gameId),
        };

        sendMessage(helloMessage);
    };

    socket.onmessage = (event) => {
        logLine("Received: " + event.data);

        let obj;
        try {
            obj = JSON.parse(event.data);
        } catch {
            return;
        }

        for (const key in obj) {
            switch (key) {
                case "setupinfo":
                    applySetupInfo(obj[key]);
                    break;
                case "snapshot":
                    applySnapshot(obj[key]);
                    break;
                case "actionresult":
                    applyActionResult(obj[key]);
                    break;
                case "readytostart":
                    if (typeof obj.connectiontoken === "string" && obj.connectiontoken.length > 0) {
                        storeConnectionToken(myUserId, gameId, obj.connectiontoken);
                    }
                    if (obj.success === true && obj[key] === false) {
                        showMessage("Waiting for opponent to join...", "info");
                    }
                    break;
                case "rematchrequest":
                    handleRematchRequest(obj[key]);
                    break;
                case "rematchstart":
                    handleRematchStart();
                    break;
                case "error":
                    showMessage(`Error: ${obj[key]}`, "error");
                    break;
            }
        }
    };

    socket.onerror = () => {
        setConnectionStatus("disconnected");
        showMessage("Connection error", "error");
        logLine("WebSocket error");
    };

    socket.onclose = () => {
        setConnectionStatus("disconnected");
        logLine("WebSocket closed");
        socket = null;
    };
});

// === Rematch and Home Button Handlers ===
rematchBtn.addEventListener("click", () => {
    if (!myUserId || !gameId) return;
    
    rematchRequested = true;
    updateRematchButton();
    
    const message = {
        gameid: gameId,
        userid: myUserId,
        sessionaction: {
            type: "rematch"
        }
    };
    
    sendMessage(message);
    showMessage("Rematch requested", "info");
});

homeBtn.addEventListener("click", () => {
    // Reload the page to go back to connection screen
    window.location.reload();
});

function handleRematchRequest(data) {
    opponentWantsRematch = true;
    updateRematchButton();
    showMessage("Opponent wants a rematch!", "info");
}

function handleRematchStart() {
    // Reset game state
    rematchRequested = false;
    opponentWantsRematch = false;
    hideGameOver();
    updateRematchButton();
    placedShipIds.clear();
    placedPlaneIds.clear();
    selectedVehicle = null;
    activeAbility = null;
    antiAircraftMode = false;
    rotation = 0;
    lastPhase = null;

    // Clear grids
    ownGrid.innerHTML = "";
    oppGrid.innerHTML = "";

    // Clear fleet panels
    yourFleetList.innerHTML = "";
    opponentFleetList.innerHTML = "";

    showMessage("Rematch starting!", "success");
}

// === Apply Server Data ===
function applySetupInfo(setupInfo) {
    lastSetupInfo = setupInfo;
    placedShipIds.clear();
    lastPhase = setupInfo.phase || "setup";

    // Switch views
    connectSection.classList.add("hidden");
    gameSection.classList.remove("hidden");

    // Populate info
    meSpan.textContent = setupInfo.you || "�";
    opponentSpan.textContent = setupInfo.opponent || "�";
    updatePhaseDisplay(lastPhase);

    // Build grids
    buildGrids(setupInfo.boardrows, setupInfo.boardcols);

    // Render initial fleet panels from vehicleview
    const vehicleView = setupInfo.userview?.vehicleview;
    if (vehicleView) {
        updateFleetPanels(vehicleView);
        drawVehiclesOnGrid(vehicleView, setupInfo.boardcols || 10);
        // Auto-select first unplaced vehicle
        selectNextUnplacedVehicle(vehicleView);
    }

    showMessage("Game started! Click a ship to select it, then click on your grid to place it.", "success");
}

function applySnapshot(snapshot) {
    if (!snapshot) return;

    // A confirmed server snapshot supersedes any transient hover/ability previews
    clearPreview();
    clearAbilityPreview();

    const myUserId = lastSetupInfo?.you;
    const currentPhase = snapshot.phase;
    
    // Check for game end BEFORE updating lastPhase
    if (currentPhase === "finished" && lastPhase !== "finished") {
        showGameOver(snapshot.winner === myUserId);
    }

    // Now update the phase tracking
    lastPhase = currentPhase;

    updatePhaseDisplay(currentPhase);
    updateTurnIndicator(snapshot.currentturn, myUserId);
    updateReadyStatus(snapshot.youready, snapshot.opponentready);

    // Clear all state classes from grids (but not preview classes)
    const stateClasses = [
        "ship", "hit", "miss", 
        "revealedmiss", "revealedhit", "scannedpositive",
        "plane", "plane-color-0", "plane-color-1", "plane-color-2", 
        "plane-color-3", "plane-color-4", "plane-color-5"
    ];
    for (const cell of ownGrid.children) {
        cell.classList.remove(...stateClasses);
    }
    for (const cell of oppGrid.children) {
        cell.classList.remove(...stateClasses);
    }

    const cols = lastSetupInfo?.boardcols || 10;

    // Apply own grid state
    const ownGridData = snapshot.userview?.boardview?.owngrid || [];
    for (const entry of ownGridData) {
        const r = entry.coord?.row;
        const c = entry.coord?.col;
        const state = entry.state;

        if (typeof r !== "number" || typeof c !== "number") continue;

        const idx = r * cols + c;
        const cell = ownGrid.children[idx];

        if (cell && state) {
            cell.classList.add(state);
        }
    }

    // Apply opponent grid state
    const oppGridData = snapshot.userview?.boardview?.opponentgrid || [];
    for (const entry of oppGridData) {
        const r = entry.coord?.row;
        const c = entry.coord?.col;
        const state = entry.state;

        if (typeof r !== "number" || typeof c !== "number") continue;

        const idx = r * cols + c;
        const cell = oppGrid.children[idx];

        if (cell && state) {
            cell.classList.add(state);
        }
    }

    // Update fleet panels from vehicleview
    const vehicleView = snapshot.userview?.vehicleview;
    if (vehicleView) {
        if (lastSetupInfo && snapshot.hasantiaircraft !== undefined) {
            lastSetupInfo.hasantiaircraft = snapshot.hasantiaircraft;
        }
        updateFleetPanels(vehicleView);
        drawVehiclesOnGrid(vehicleView, cols);
    }
}

function applyActionResult(result) {
    switch (result.type) {
        case "fireresult":
            applyFireResult(result);
            break;
        case "fireantiaircraft":
            applyFireAntiAircraftResult(result);
            break;
        case "placeshipresult":
            applyPlaceShipResult(result);
            break;
        case "placeplaneresult":
            applyPlacePlaneResult(result);
            break;
        case "readyresult":
            applyReadyResult(result);
            break;
        case "transientoverlayresult":
            applyTransientOverlayResult(result);
            break;
        case "activateabilityresult":
            applyActivateAbilityResult(result);
            break;
    }
}

function applyFireResult(result) {
    if (result.success) {
        // Compare actinguser to determine if we fired or were fired upon
        const iDidThis = result.actinguser === myUserId;

        if (result.data?.ishit) {
            if (result.data.issunk) {
                if (iDidThis) {
                    showMessage(`You sank their ${result.data.sunkname}!`, "success");
                } else {
                    showMessage(`Your ${result.data.sunkname} was sunk!`, "error");
                }
                // Fleet panels will be updated by the snapshot that follows
            } else {
                if (iDidThis) {
                    showMessage("Hit!", "success");
                } else {
                    showMessage("Your ship was hit!", "error");
                }
            }
        } else {
            if (iDidThis) {
                showMessage("Miss", "info");
            } else {
                showMessage(`${result.actinguser} missed!`, "info");
            }
        }
    } else {
        const errorMessages = {
            notyourturn: "It's not your turn",
            invalidplacement: "You can't fire there",
        };
        showMessage(errorMessages[result.error] || "Unable to fire", "error");
    }
}

function applyFireAntiAircraftResult(result) {
    if (result.success) {
        const iDidThis = result.actinguser === myUserId;

        if (result.data?.ishit) {
            if (result.data.isdestroyed) {
                if (iDidThis) {
                    showMessage(`You destroyed their ${result.data.destroyedname}!`, "success");
                } else {
                    showMessage(`Your ${result.data.destroyedname} was shot down!`, "error");
                }
            } else {
                if (iDidThis) {
                    showMessage("Anti-Aircraft hit!", "success");
                } else {
                    showMessage("Your plane was hit!", "error");
                }
            }
        } else {
            if (iDidThis) {
                showMessage("Anti-Aircraft miss - no planes at that location", "info");
            } else {
                showMessage(`${result.actinguser} fired anti-aircraft and missed!`, "info");
            }
        }
    } else {
        const errorMessages = {
            notyourturn: "It's not your turn",
            invalidplacement: "You can't target there",
        };
        showMessage(errorMessages[result.error] || "Unable to fire anti-aircraft", "error");
    }
}

function applyPlaceShipResult(result) {
    if (result.success) {
        // Mark this ship as placed and select next unplaced vehicle
        if (selectedVehicle && selectedVehicle.type === 'ship') {
            placedShipIds.add(selectedVehicle.id);
        }
        if (lastSetupInfo && lastSetupInfo.vehicleview) {
            selectNextUnplacedVehicle(lastSetupInfo.vehicleview);
        }
    } else {
        const errorMessages = {
            wrongphase: "Ships can only be placed during setup",
            invalidplacement: "Invalid placement - ships cannot overlap or go out of bounds",
            shipnotfound: "Ship not found",
        };
        showMessage(errorMessages[result.error] || "Unable to place ship", "error");
    }
}

function applyPlacePlaneResult(result) {
    if (result.success) {
        // Mark this plane as placed and select next unplaced vehicle
        if (selectedVehicle && selectedVehicle.type === 'plane') {
            placedPlaneIds.add(selectedVehicle.id);
        }
        if (lastSetupInfo && lastSetupInfo.vehicleview) {
            selectNextUnplacedVehicle(lastSetupInfo.vehicleview);
        }
    } else {
        const errorMessages = {
            wrongphase: "Planes can only be placed during setup",
            invalidplacement: "Invalid placement - planes must be placed on a carrier",
            vehiclenotfound: "Plane not found",
        };
        showMessage(errorMessages[result.error] || "Unable to place plane", "error");
    }
}

function applyReadyResult(result) {
    if (result.success) {
        // Don't show "waiting" message here - let the snapshot handle the phase transition
        // The snapshot will tell us if both players are ready
    } else {
        const errorMessages = {
            invalidplacement: "Place all ships before readying up",
            notyourturn: "Your fleet placement is invalid",
        };
        showMessage(errorMessages[result.error] || "Unable to ready up", "error");
    }
}

function applyTransientOverlayResult(result) {
    if (!result.success) return;

    const data = result.data;
    if (!data || !data.overlay) return;

    const cols = lastSetupInfo?.boardcols || 10;
    const myUserId = lastSetupInfo?.you;

    // Clear any existing preview
    clearPreview();

    // Process the overlay map
    for (const coordKey in data.overlay) {
        // Parse coordinate key "row,col,userId"
        const parts = coordKey.split(",");
        if (parts.length !== 3) continue;

        const r = parseInt(parts[0], 10);
        const c = parseInt(parts[1], 10);
        const userId = parts[2];

        if (typeof r !== "number" || typeof c !== "number" || isNaN(r) || isNaN(c)) continue;

        const states = data.overlay[coordKey];
        if (!Array.isArray(states) || states.length === 0) continue;

        // Only show preview for in-bounds cells
        if (r < 0 || c < 0) continue;

        // Determine which grid to apply to based on userId
        const targetGrid = (userId === myUserId) ? ownGrid : oppGrid;

        const idx = r * cols + c;
        const cell = targetGrid.children[idx];
        if (!cell) continue;

        // Apply CSS classes based on the states
        // Priority: invalidPlacement > torpedo directions > validPlacement > targetedSquare
        if (states.includes("invalidplacement")) {
            cell.classList.add("preview-invalid");
        } else if (states.includes("torpedoup")) {
            cell.classList.add("torpedoup");
        } else if (states.includes("torpedodown")) {
            cell.classList.add("torpedodown");
        } else if (states.includes("torpedoleft")) {
            cell.classList.add("torpedoleft");
        } else if (states.includes("torpedoright")) {
            cell.classList.add("torpedoright");
        } else if (states.includes("validplacement")) {
            cell.classList.add("preview-valid");
        } else if (states.includes("targetedsquare")) {
            cell.classList.add("preview-targeted");
        }
    }
}

function applyActivateAbilityResult(result) {
    const iDidThis = result.actinguser === myUserId;

    if (result.success) {
        const innerData = result.data?.data; // ActivateAbilityResult wraps data in { success, error, data }
        const resultType = innerData?.resulttype;

        // Handle different ability result types
        if (innerData?.ishit !== undefined) {
            // Torpedo or bulk fire result (exocet, apache, tomahawk)
            const abilityName = getAbilityDisplayName(resultType) || "Ability";
            if (innerData.ishit) {
                if (iDidThis) {
                    showMessage(`${abilityName} hit!`, "success");
                } else {
                    showMessage(`${result.actinguser} fired their ${abilityName} and hit your fleet!`, "error");
                }
            } else {
                if (iDidThis) {
                    showMessage(`${abilityName} missed`, "info");
                } else {
                    showMessage(`${result.actinguser} fired their ${abilityName} and missed!`, "info");
                }
            }
        } else if (innerData?.isfound !== undefined) {
            // Scan result
            if (iDidThis) {
                if (innerData.isfound) {
                    showMessage("Scan detected enemy ships in the area!", "success");
                } else {
                    showMessage("Scan found no ships in the area", "info");
                }
            } else {
                if (innerData.isfound) {
                    showMessage(`${result.actinguser} used a Scan and detected ships!`, "error");
                } else {
                    showMessage(`${result.actinguser} used a Scan and found nothing`, "info");
                }
            }
        } else if (innerData?.hitsrevealed !== undefined) {
            // Reveal result
            const count = innerData.hitsrevealed?.length || 0;
            if (iDidThis) {
                if (count > 0) {
                    showMessage(`Revealed ${count} ship position${count !== 1 ? "s" : ""}!`, "success");
                } else {
                    showMessage("No ships found in revealed area", "info");
                }
            } else {
                if (count > 0) {
                    showMessage(`${result.actinguser} used Reveal and found ${count} of your ship position${count !== 1 ? "s" : ""}!`, "error");
                } else {
                    showMessage(`${result.actinguser} used Reveal but found nothing`, "info");
                }
            }
        } else if (resultType === "relocate") {
            const vehicleId = innerData?.shipid;
            const vehicleView = lastSetupInfo?.vehicleview;
            const fleet = iDidThis ? vehicleView?.yourfleet : vehicleView?.opponentfleet;
            let vehicleName = null;
            if (vehicleId !== undefined) {
                for (const s of fleet?.ships || []) {
                    if (s.id === vehicleId) { vehicleName = s.name; break; }
                }
                if (!vehicleName) {
                    for (const pl of fleet?.planes || []) {
                        if (pl.id === vehicleId) { vehicleName = pl.name; break; }
                    }
                }
            }
            if (iDidThis) {
                showMessage(`${vehicleName ?? "Vehicle"} relocated successfully`, "success");
            } else {
                showMessage(`${result.actinguser} relocated their ${vehicleName ?? "vehicle"}!`, "info");
            }
        } else {
            if (iDidThis) {
                showMessage("Ability activated successfully", "success");
            }
        }
    } else {
        const errorMessages = {
            notyourturn: "It's not your turn",
            notyourship: "That's not your ship",
            shipsunk: "That ship is sunk and cannot use abilities",
            nosuchability: "That vehicle doesn't have this ability",
            outofbounds: "Target is out of bounds",
        };
        showMessage(errorMessages[result.error] || "Unable to activate ability", "error");
    }
}
