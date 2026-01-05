import "./style.css"

// Grab references to HTML elements by id
const userIdInput = document.getElementById("userid");
const gameIdInput = document.getElementById("gameid");
const connectBtn = document.getElementById("connectBtn");
const statusSpan = document.getElementById("status");
const logPre = document.getElementById("log");

const connectSection = document.getElementById("connect");
const gameSection = document.getElementById("game");

const meSpan = document.getElementById("me");
const opponentSpan = document.getElementById("opponent");
const phaseSpan = document.getElementById("phase");

const shipSelect = document.getElementById("shipSelect");
const rotateBtn = document.getElementById("rotateBtn");
const readyBtn = document.getElementById("readyBtn");

const ownGrid = document.getElementById("ownGrid");

// Helper: append a line to the log
function logLine(text) {
    logPre.textContent += text + "\n";
}

// WebSocket reference (null until connected)
let socket = null;

let rotation = 0; //number of counterclockwise rotations
let lastGameDetails = null;

rotateBtn.addEventListener("click", () => {
    rotation = (rotation + 1) % 4;
});

function showGameScreen(details) {
    connectSection.style.display = "none";
    gameSection.style.display = "block";

    meSpan.textContent = details.you ?? "";
    opponentSpan.textContent = details.opponent ?? "";
    phaseSpan.textContent = details.phase ?? "";

    // Populate ships
    shipSelect.innerHTML = "";
    for (const ship of details.fleet ?? []) {
        const opt = document.createElement("option");
        opt.value = String(ship.id);
        opt.textContent = `${ship.name} (#${ship.id})`;
        shipSelect.appendChild(opt);
    }

    // Build grid
    const rows = details.boardrows;
    const cols = details.boardcols;

    ownGrid.style.setProperty("--rows", rows);
    ownGrid.style.setProperty("--cols", cols);

    ownGrid.innerHTML = "";
    for (let r = 0; r < rows; r++) {
        for (let c = 0; c < cols; c++) {
            const cell = document.createElement("div");
            cell.className = "cell";
            cell.dataset.row = String(r);
            cell.dataset.col = String(c);

            cell.addEventListener("click", () => {
                const shipIdStr = shipSelect.value;
                if (!shipIdStr) return;

                // Send a placeship action
                // {"gameid":"g1","userid":"p1","sessionaction":{"type":"placeship","data":{"position":{"row":0,"col":0},"rotation":0,"shipid":20}}}
                const userId = userIdInput.value;
                const gameId = gameIdInput.value;

                const placeShip = {
                    gameid: gameId,
                    userid: userId,
                    sessionaction: {
                        type: "placeship",
                        data: {
                            position: { row: r, col: c },
                            rotation: rotation,
                            shipid: Number(shipIdStr),
                        },
                    },
                };

                socket.send(JSON.stringify(placeShip));
                logLine("Sent: " + JSON.stringify(placeShip));
            });

            ownGrid.appendChild(cell);
        }
    }

    // Initial render of any existing cells from userview.boardview.owngrid
    applyUserView(details.userview);
}

function applyUserView(userview) {
    if (!userview) return;

    phaseSpan.textContent = lastGameDetails?.phase ?? phaseSpan.textContent;

    // Clear ship classes
    for (const cell of ownGrid.children) {
        cell.classList.remove("ship");
    }

    const own = userview.boardview?.owngrid ?? [];
    for (const entry of own) {
        const r = entry.coord?.row;
        const c = entry.coord?.col;
        const state = entry.state;

        if (typeof r !== "number" || typeof c !== "number") continue;
        if (state !== "ship") continue; // MVP: only draw ships for now

        const idx = r * (lastGameDetails.boardcols) + c;
        const cell = ownGrid.children[idx];
        if (cell) cell.classList.add("ship");
    }
}

function looksLikeGameDetails(obj) {
    return obj &&
        typeof obj === "object" &&
        typeof obj.boardrows === "number" &&
        typeof obj.boardcols === "number" &&
        Array.isArray(obj.fleet) &&
        typeof obj.you === "string" &&
        typeof obj.opponent === "string" &&
        typeof obj.phase === "string" &&
        obj.userview;
}

// What happens when the user clicks "Connect"
connectBtn.addEventListener("click", () => {
    const userId = userIdInput.value;
    const gameId = gameIdInput.value;

    if (!userId || !gameId) {
        alert("Please enter both User ID and Game ID");
        return;
    }

    statusSpan.textContent = "connecting...";

    socket = new WebSocket("wss://remoteterminal.titohq.com/ws");

    socket.onopen = () => {
        statusSpan.textContent = "connected";
        logLine("WebSocket connected");

        const helloMessage = {
            type: "hello",
            userid: userId,
            gameid: gameId
        };

        socket.send(JSON.stringify(helloMessage));
        logLine("Sent hello: " + JSON.stringify(helloMessage));
    };

    socket.onmessage = (event) => {
        logLine("Received: " + event.data);

        let obj = null;
        try {
            obj = JSON.parse(event.data);
        } catch {
            return;
        }

        // If it's the big game details message, switch screens
        if (looksLikeGameDetails(obj)) {
            lastGameDetails = obj;
            showGameScreen(obj);
            return;
        }

        // Otherwise, if server later sends snapshots/results, handle them here.
        // MVP: if it contains a "userview" field, update our grid.
        if (obj && obj.snapshot && lastGameDetails) {
            // allow server to send partial updates that include userview + maybe phase
            if (typeof obj.phase === "string") lastGameDetails.phase = obj.phase;
            lastGameDetails.userview = obj.snapshot.userview;
            applyUserView(obj.snapshot.userview);
            return;
        }
    };

    socket.onerror = () => {
        statusSpan.textContent = "error";
        logLine("WebSocket error");
    };

    socket.onclose = () => {
        statusSpan.textContent = "disconnected";
        logLine("WebSocket closed");
    };
});

readyBtn.addEventListener("click", () => {
    //{"gameid":"g1","userid":"p1","sessionaction":{"type":"ready","data":null}}
    const userId = userIdInput.value;
    const gameId = gameIdInput.value;

    const readyUp = {
        gameid: gameId,
        userid: userId,
        sessionaction: {
            type: "ready",
            data: null,
        },
    };

    socket.send(JSON.stringify(readyUp));
    logLine("Sent: " + JSON.stringify(readyUp));
});
