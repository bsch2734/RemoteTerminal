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

const ownGrid = document.getElementById("ownGrid");
const oppGrid = document.getElementById("oppGrid");

// === State ===
let socket = null;
let rotation = 0;
let lastSetupInfo = null;
let messageTimeout = null;

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
    }, 4000);
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

// === Grid Building ===
function buildGrids(rows, cols) {
    ownGrid.style.setProperty("--rows", rows);
    ownGrid.style.setProperty("--cols", cols);
    oppGrid.style.setProperty("--rows", rows);
    oppGrid.style.setProperty("--cols", cols);

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

// === Event Handlers ===
rotateBtn.addEventListener("click", () => {
    rotation = (rotation + 1) % 4;
    const directions = ["?", "?", "?", "?"];
    showMessage(`Rotation: ${directions[rotation]}`, "info");
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

    // Switch views
    connectSection.classList.add("hidden");
    gameSection.classList.remove("hidden");

    // Populate info
    meSpan.textContent = setupInfo.you || "—";
    opponentSpan.textContent = setupInfo.opponent || "—";
    updatePhaseDisplay(setupInfo.phase || "setup");

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

    updatePhaseDisplay(snapshot.phase);
    updateTurnIndicator(snapshot.currentturn, lastSetupInfo?.you);
    updateReadyStatus(snapshot.youready, snapshot.opponentready);

    // Clear all state classes from grids
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
    }
}

function applyFireResult(result) {
    if (result.success) {
        if (result.data?.ishit) {
            if (result.data.issunk) {
                showMessage(`?? Hit! You sank their ${result.data.sunkname}!`, "success");
            } else {
                showMessage("?? Hit!", "success");
            }
        } else {
            showMessage("?? Miss!", "info");
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
    if (!result.success) {
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
        showMessage("You're ready! Waiting for opponent...", "success");
    } else {
        const errorMessages = {
            invalidplacement: "Place all ships before readying up",
            notyourturn: "Your fleet placement is invalid",
        };
        showMessage(errorMessages[result.error] || "Unable to ready up", "error");
    }
}
