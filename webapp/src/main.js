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
const shipSelect = document.getElementById("shipSelect");
const rotateBtn = document.getElementById("rotateBtn");
const readyBtn = document.getElementById("readyBtn");

const messageToast = document.getElementById("messageToast");
const messageText = document.getElementById("messageText");

const gameOverOverlay = document.getElementById("gameOverOverlay");
const gameOverTitle = document.getElementById("gameOverTitle");
const gameOverMessage = document.getElementById("gameOverMessage");

const ownGrid = document.getElementById("ownGrid");
const oppGrid = document.getElementById("oppGrid");
const ownColLabels = document.getElementById("ownColLabels");
const ownRowLabels = document.getElementById("ownRowLabels");
const oppColLabels = document.getElementById("oppColLabels");
const oppRowLabels = document.getElementById("oppRowLabels");

// === State ===
let socket = null;
let rotation = 0;
let lastSetupInfo = null;
let messageTimeout = null;
let lastPhase = null;
let placedShipIds = new Set();

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
    } else {
        setupControls.classList.add("hidden");
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
            cell.addEventListener("mouseleave", () => clearPreview());
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
            oppGrid.appendChild(cell);
        }
    }
}

// === Click Handlers ===
function handleOwnGridClick(row, col) {
    const shipIdStr = shipSelect.value;
    if (!shipIdStr || !lastSetupInfo) return;

    const message = {
        gameid: lastSetupInfo.gameid,
        userid: lastSetupInfo.you,
        sessionaction: {
            type: "placeship",
            data: {
                position: { row, col },
                rotation: rotation,
                shipid: Number(shipIdStr),
            },
        },
    };

    sendMessage(message);
}

function handleOwnGridHover(row, col) {
    const shipIdStr = shipSelect.value;
    if (!shipIdStr || !lastSetupInfo) return;
    
    // Only show preview during setup phase
    if (lastPhase !== "setup") return;

    const message = {
        gameid: lastSetupInfo.gameid,
        userid: lastSetupInfo.you,
        sessionaction: {
            type: "checkplacement",
            data: {
                position: { row, col },
                rotation: rotation,
                shipid: Number(shipIdStr),
            },
        },
    };

    sendMessage(message);
}

function clearPreview() {
    // Clear preview overlay classes from all cells
    for (const cell of ownGrid.children) {
        cell.classList.remove("preview-valid", "preview-invalid");
    }
}

function handleOppGridClick(row, col) {
    if (!lastSetupInfo) return;

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

function selectNextShip(currentShipId) {
    placedShipIds.add(currentShipId);
    
    // Find next unplaced ship
    const options = Array.from(shipSelect.options);
    for (const opt of options) {
        const id = Number(opt.value);
        if (!placedShipIds.has(id)) {
            shipSelect.value = opt.value;
            return;
        }
    }
}

// === Event Handlers ===
rotateBtn.addEventListener("click", () => {
    rotation = (rotation + 1) % 4;
    const directions = ["Right", "Up", "Left", "Down"];
    showMessage(`Rotation: ${directions[rotation]}`, "info");
});

// Keyboard shortcut for rotation
document.addEventListener("keydown", (e) => {
    if (e.key === "r" || e.key === "R") {
        // Don't trigger if user is typing in an input
        if (e.target.tagName === "INPUT" || e.target.tagName === "TEXTAREA") return;
        if (lastPhase !== "setup") return;
        
        rotateBtn.click();
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

connectBtn.addEventListener("click", () => {
    const userId = userIdInput.value.trim();
    const gameId = gameIdInput.value.trim();

    if (!userId || !gameId) {
        showMessage("Please enter both User ID and Game ID", "error");
        return;
    }

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
            userid: userId,
            gameid: gameId,
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
                case "waiting":
                    showMessage("Waiting for opponent to join...", "info");
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
    };
});

// === Apply Server Data ===
function applySetupInfo(setupInfo) {
    lastSetupInfo = setupInfo;
    placedShipIds.clear();
    lastPhase = setupInfo.phase || "setup";

    // Switch views
    connectSection.classList.add("hidden");
    gameSection.classList.remove("hidden");

    // Populate info
    meSpan.textContent = setupInfo.you || "—";
    opponentSpan.textContent = setupInfo.opponent || "—";
    updatePhaseDisplay(lastPhase);

    // Populate ship selector
    shipSelect.innerHTML = "";
    for (const ship of setupInfo.fleet || []) {
        const opt = document.createElement("option");
        opt.value = String(ship.id);
        opt.textContent = ship.name;
        shipSelect.appendChild(opt);
    }

    // Build grids
    buildGrids(setupInfo.boardrows, setupInfo.boardcols);

    showMessage("Game started! Place your ships.", "success");
}

function applySnapshot(snapshot) {
    if (!snapshot) return;

    const myUserId = lastSetupInfo?.you;
    const currentPhase = snapshot.phase;
    
    // Check for game end BEFORE updating lastPhase
    if (currentPhase === "finished" && lastPhase !== "finished") {
        // Determine if we won by checking if we have any unhit ships left
        const ownGridData = snapshot.userview?.boardview?.owngrid || [];
        const hasShipsLeft = ownGridData.some(entry => entry.state === "ship");
        showGameOver(hasShipsLeft);
    }
    
    // Now update the phase tracking
    lastPhase = currentPhase;
    
    updatePhaseDisplay(currentPhase);
    updateTurnIndicator(snapshot.currentturn, myUserId);
    updateReadyStatus(snapshot.youready, snapshot.opponentready);

    // Clear all state classes from grids (but not preview classes)
    for (const cell of ownGrid.children) {
        cell.classList.remove("ship", "hit", "miss");
    }
    for (const cell of oppGrid.children) {
        cell.classList.remove("ship", "hit", "miss");
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
        if (cell && state) cell.classList.add(state);
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
        if (cell && state) cell.classList.add(state);
    }
}

function applyActionResult(result) {
    switch (result.type) {
        case "fireresult":
            applyFireResult(result);
            break;
        case "placeshipresult":
            applyPlaceShipResult(result);
            break;
        case "readyresult":
            applyReadyResult(result);
            break;
        case "checkplacementresult":
            applyCheckPlacementResult(result);
            break;
    }
}

function applyFireResult(result) {
    if (result.success) {
        const isMyShot = result.data?.ishit !== undefined;
        
        // Both players receive this result - determine who fired
        // If hitid matches one of our fleet ships, we were hit (opponent fired)
        // Otherwise we fired
        const hitId = result.data?.hitid;
        const myFleetIds = (lastSetupInfo?.fleet || []).map(s => s.id);
        const wasHitOnMe = hitId !== undefined && myFleetIds.includes(hitId);
        
        if (result.data?.ishit) {
            if (result.data.issunk) {
                if (wasHitOnMe) {
                    showMessage(`Your ${result.data.sunkname} was sunk!`, "error");
                } else {
                    showMessage(`You sank their ${result.data.sunkname}!`, "success");
                }
            } else {
                if (wasHitOnMe) {
                    showMessage("Your ship was hit!", "error");
                } else {
                    showMessage("Hit!", "success");
                }
            }
        } else {
            // Miss - only show for the shooter (they get miss, target gets nothing meaningful)
            if (!wasHitOnMe) {
                showMessage("Miss", "info");
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

function applyPlaceShipResult(result) {
    if (result.success) {
        // Select the next ship automatically
        const currentShipId = Number(shipSelect.value);
        selectNextShip(currentShipId);
    } else {
        const errorMessages = {
            wrongphase: "Ships can only be placed during setup",
            invalidplacement: "Invalid placement - ships cannot overlap or go out of bounds",
            shipnotfound: "Ship not found",
        };
        showMessage(errorMessages[result.error] || "Unable to place ship", "error");
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

function applyCheckPlacementResult(result) {
    if (!result.success) return;
    
    const data = result.data;
    if (!data) return;
    
    const cols = lastSetupInfo?.boardcols || 10;
    const previewClass = data.valid ? "preview-valid" : "preview-invalid";
    
    // Clear any existing preview
    clearPreview();
    
    // Apply preview overlay to the specified coords
    for (const coord of data.coords || []) {
        const r = coord.row;
        const c = coord.col;
        
        if (typeof r !== "number" || typeof c !== "number") continue;
        
        // Only show preview for in-bounds cells
        if (r < 0 || c < 0) continue;
        
        const idx = r * cols + c;
        const cell = ownGrid.children[idx];
        if (cell) {
            cell.classList.add(previewClass);
        }
    }
}
