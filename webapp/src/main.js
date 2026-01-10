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
const oppGrid = document.getElementById("oppGrid");

// Helper: append a line to the log
function logLine(text) {
    logPre.textContent += text + "\n";
}

// WebSocket reference (null until connected)
let socket = null;

let rotation = 0; //number of counterclockwise rotations
let lastsetupinfo = null;

rotateBtn.addEventListener("click", () => {
    rotation = (rotation + 1) % 4;
});

function applySetupInfo(setupinfo) {
    lastsetupinfo = setupinfo;

    // show game/ hide connect screen
    connectSection.style.display = "none";
    gameSection.style.display = "block";

    // Populate game info
    meSpan.textContent = lastsetupinfo.you ?? "";
    opponentSpan.textContent = lastsetupinfo.opponent ?? "";
    phaseSpan.textContent = lastsetupinfo.phase ?? "";

    // Populate ships
    shipSelect.innerHTML = "";
    for (const ship of lastsetupinfo.fleet ?? []) {
        const opt = document.createElement("option");
        opt.value = String(ship.id);
        opt.textContent = `${ship.name}`;
        shipSelect.appendChild(opt);
    }

    // Build grid
    const rows = lastsetupinfo.boardrows;
    const cols = lastsetupinfo.boardcols;

    ownGrid.style.setProperty("--rows", rows);
    ownGrid.style.setProperty("--cols", cols);

    ownGrid.innerHTML = "";
    //build ownGrid
    for (let r = 0; r < rows; r++) {
        for (let c = 0; c < cols; c++) {
            const cell = document.createElement("div");
            cell.className = "cell";

            cell.addEventListener("click", () => {
                const shipIdStr = shipSelect.value;
                if (!shipIdStr) return;

                // Send a placeship action
                const userId = lastsetupinfo.you;
                const gameId = lastsetupinfo.gameid;

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

    //build oppGrid
    oppGrid.innerHTML = "";

    oppGrid.style.setProperty("--rows", rows);
    oppGrid.style.setProperty("--cols", cols);
    for (let r = 0; r < rows; r++) {
        for (let c = 0; c < cols; c++) {
            const cell = document.createElement("div");
            cell.className = "cell";

            cell.addEventListener("click", () => {
                // Send a fire action
                const userId = lastsetupinfo.you;
                const gameId = lastsetupinfo.gameid;

                const fire = {
                    gameid: gameId,
                    userid: userId,
                    sessionaction: {
                        type: "fire",
                        data: {
                            target: { row: r, col: c }
                        },
                    },
                };
                socket.send(JSON.stringify(fire));
                logLine("Sent: " + JSON.stringify(fire));
            });
            oppGrid.appendChild(cell);
        }
    }
}

function applySnapshot(snapshot) {
    if (!snapshot) return;

    //display current phase
    phaseSpan.textContent = snapshot.phase;

    //display whose turn it is
    //todo: make an HTML element to show the current turn

    //display ready status for each player
    //todo: make HTML elements to show the ready statuses

    // Clear ship classes
    for (const cell of ownGrid.children) {
        cell.classList.remove("ship");
    }

    const own = snapshot.userview.boardview?.owngrid ?? [];
    for (const entry of own) {
        const r = entry.coord?.row;
        const c = entry.coord?.col;
        const state = entry.state;

        if (typeof r !== "number" || typeof c !== "number") continue;

        const idx = r * (lastsetupinfo.boardcols) + c;
        const cell = ownGrid.children[idx];
        if (cell) cell.classList.add(state);
    }

    const opp = snapshot.userview.boardview?.opponentgrid ?? [];
    for (const entry of opp) {
        const r = entry.coord?.row;
        const c = entry.coord?.col;
        const state = entry.state;

        if (typeof r !== "number" || typeof c !== "number") continue;
        
        const idx = r * (lastsetupinfo.boardcols) + c;
        const cell = oppGrid.children[idx];
        if (cell) cell.classList.add(state);
    }
}

function applyActionResult(actionresult){
    switch (actionresult.type) {
        case "fireresult":
            applyFireActionResult(actionresult);
            break;
        case "placeshipresult":
            applyPlaceShipActionResult(actionresult);
            break;
        case "readyresult":
            applyReadyActionResult(actionresult);
            break;
    }
}

function applyFireActionResult(fireactionresult) {
    if (fireactionresult.success) {
        if (fireactionresult.data.ishit) {
            "Hit!\n"; //todo: make a place to post the messages
            if (fireactionresult.data.issunk) {
                "You sank their " + fireactionresult.data.sunkname + "\n"; //todo: make a place to post the messages
            }
        }
    } else {
        switch (fireactionresult.error) {
            case ("notyourturn"):
                "It's not your turn"; //todo: make a place to post the messages
                break;
            case ("invalidplacement"):
                "You can't fire there"; //todo: make a place to post the messages
                break;
            default:
                "Unknown error while firing"; //todo: make a place to post the messages
                break;
        }
    }
}

function applyPlaceShipActionResult(placeshipactionresult) {
    if (placeshipactionresult.success) {
        //no need to log anything, there is visual feedback
    } else {
        switch (placeshipactionresult.error) {
            case ("wrongphase"):
                "You can only place ships during setup"; //todo: make a place to post the messages
                break;
            case ("invalidplacement"):
                "You can't place this ship there"; //todo: make a place to post the messages
                break;
            case ("shipnotfound"):
                "You don't own that ship"; //todo: make a place to post the messages
                break;
            default:
                "Unknown error while placing a ship"; //todo: make a place to post the messages
                break;                
        }
    }
}

function applyReadyActionResult(applyreadyactionresult) {
    if (applyreadyactionresult.success) {
        readyBtn.classList.add("isready")
    } else {
        switch (applyreadyactionresult.error) {
            case ("invalidplacement"):
                "You must place all of your ships to ready up"; //todo: make a place to post the messages
                break;
            case ("notyourturn"):
                "Your fleet is placed but not in a valid state"; //todo: make a place to post the messages
                break;
            default:
                "Unknown error while readying up"; //todo: make a place to post the messages
                break;
        }
    }
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

    const WS_URL = import.meta.env.VITE_WS_URL;
    if (!WS_URL) {
        throw new Error("Missing VITE_WS_URL. Create webapp/.env.local (see .env.example).");
    }
    socket = new WebSocket(WS_URL);

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

        for (const key in obj) {
            switch (key) {
                case "actionresult":
                    applyActionResult(obj[key]);
                    break;
                case "snapshot":
                    applySnapshot(obj[key]);
                    break;
                case "setupinfo":
                    applySetupInfo(obj[key]);
                    break;
            }
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
