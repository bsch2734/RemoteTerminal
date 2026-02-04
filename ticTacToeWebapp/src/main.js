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
const yourSymbol = document.getElementById("yourSymbol");
const opponentSymbol = document.getElementById("opponentSymbol");
const phaseSpan = document.getElementById("phase");
const turnIndicator = document.getElementById("turnIndicator");

const messageToast = document.getElementById("messageToast");
const messageText = document.getElementById("messageText");

const gameOverOverlay = document.getElementById("gameOverOverlay");
const gameOverTitle = document.getElementById("gameOverTitle");
const gameOverMessage = document.getElementById("gameOverMessage");

const board = document.getElementById("board");
const cells = board.querySelectorAll(".cell");

// === State ===
let socket = null;
let messageTimeout = null;
let lastPhase = null;
let myUserId = null;
let gameId = null;
let mySymbol = null; // "X" or "O"

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
}

function updateTurnIndicator(currentTurn) {
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

function playerToSymbol(player) {
    switch (player) {
        case "one": return "X";
        case "two": return "O";
        default: return "";
    }
}

function showGameOver(winner) {
    gameOverOverlay.classList.remove("hidden");
    const content = gameOverOverlay.querySelector(".game-over-content");
    
    if (winner === "none") {
        content.classList.remove("victory", "defeat");
        content.classList.add("draw");
        gameOverTitle.textContent = "Draw!";
        gameOverMessage.textContent = "The game ended in a tie.";
    } else if (winner === myUserId) {
        content.classList.add("victory");
        content.classList.remove("defeat", "draw");
        gameOverTitle.textContent = "Victory!";
        gameOverMessage.textContent = "You won the game!";
    } else {
        content.classList.add("defeat");
        content.classList.remove("victory", "draw");
        gameOverTitle.textContent = "Defeat";
        gameOverMessage.textContent = "Your opponent won.";
    }
}

// === Click Handler ===
function handleCellClick(index) {
    if (!myUserId || !gameId) return;

    const message = {
        gameid: gameId,
        userid: myUserId,
        sessionaction: {
            type: "move",
            data: {
                target: index
            }
        }
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
cells.forEach((cell, index) => {
    cell.addEventListener("click", () => handleCellClick(index));
});

connectBtn.addEventListener("click", () => {
    const userId = userIdInput.value.trim();
    const inputGameId = gameIdInput.value.trim();

    if (!userId || !inputGameId) {
        showMessage("Please enter both User ID and Game ID", "error");
        return;
    }

    myUserId = userId;
    gameId = inputGameId;

    setConnectionStatus("connecting");

    const WS_URL = import.meta.env.VITE_WS_URL;
    if (!WS_URL) {
        showMessage("Missing VITE_WS_URL configuration", "error");
        throw new Error("Missing VITE_WS_URL. Create ticTacToeWebapp/.env.local (see .env.example).");
    }

    socket = new WebSocket(WS_URL);

    socket.onopen = () => {
        setConnectionStatus("connected");
        logLine("WebSocket connected");

        const helloMessage = {
            type: "hello",
            userid: userId,
            gameid: inputGameId
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
                case "snapshot":
                    applySnapshot(obj[key]);
                    break;
                case "actionresult":
                    applyActionResult(obj[key]);
                    break;
                case "waiting":
                    showWaitingScreen();
                    break;
                case "readytostart":
                    if (obj[key]) {
                        showGameScreen();
                    }
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
function showWaitingScreen() {
    connectSection.classList.add("hidden");
    gameSection.classList.remove("hidden");
    
    meSpan.textContent = myUserId;
    opponentSpan.textContent = "Waiting...";
    
    showMessage("Waiting for opponent to join...", "info");
}

function showGameScreen() {
    connectSection.classList.add("hidden");
    gameSection.classList.remove("hidden");
    
    showMessage("Game started!", "success");
}

function applySnapshot(snapshot) {
    if (!snapshot) return;

    // Show game screen if not already visible
    connectSection.classList.add("hidden");
    gameSection.classList.remove("hidden");

    const currentPhase = snapshot.phase;
    
    // Check for game end
    if (currentPhase === "finished" && lastPhase !== "finished") {
        // Determine winner from the board state
        const winner = determineWinner(snapshot.board, snapshot.currentturn);
        showGameOver(winner);
    }
    
    lastPhase = currentPhase;
    
    updatePhaseDisplay(currentPhase);
    updateTurnIndicator(snapshot.currentturn);

    // Update board
    const boardData = snapshot.board || [];
    cells.forEach((cell, index) => {
        const player = boardData[index];
        const symbol = playerToSymbol(player);
        
        cell.textContent = symbol;
        cell.className = "cell";
        
        if (symbol === "X") {
            cell.classList.add("x");
        } else if (symbol === "O") {
            cell.classList.add("o");
        }
    });

    // Determine and display player symbols based on who is current turn
    // Player one is always X, player two is always O
    // We can infer our symbol from the currentturn and board state
    if (!mySymbol && snapshot.currentturn) {
        // If it's our turn and board is empty or has even moves, we're X (player one)
        // This is a simplification - ideally server would tell us our player number
        const filledCells = boardData.filter(p => p !== "none").length;
        if (snapshot.currentturn === myUserId) {
            mySymbol = (filledCells % 2 === 0) ? "X" : "O";
        } else {
            mySymbol = (filledCells % 2 === 0) ? "O" : "X";
        }
        
        yourSymbol.textContent = mySymbol;
        yourSymbol.className = `symbol-badge ${mySymbol.toLowerCase()}`;
        opponentSymbol.textContent = mySymbol === "X" ? "O" : "X";
        opponentSymbol.className = `symbol-badge ${mySymbol === "X" ? "o" : "x"}`;
    }
}

function determineWinner(board, lastPlayer) {
    const winPatterns = [
        [0, 1, 2], [3, 4, 5], [6, 7, 8], // rows
        [0, 3, 6], [1, 4, 7], [2, 5, 8], // cols
        [0, 4, 8], [2, 4, 6]             // diagonals
    ];

    for (const pattern of winPatterns) {
        const [a, b, c] = pattern;
        if (board[a] !== "none" && board[a] === board[b] && board[b] === board[c]) {
            // Found a winner - but we need to map player to userId
            // For now, use lastPlayer as a hint (the player who just moved)
            // This is imperfect but works for basic case
            return lastPlayer === myUserId ? myUserId : "opponent";
        }
    }

    // Check for draw (all cells filled)
    const isFull = board.every(cell => cell !== "none");
    if (isFull) {
        return "none"; // draw
    }

    return null; // game not over
}

function applyActionResult(result) {
    if (result.type === "moveresult") {
        if (!result.success) {
            const errorMessages = {
                notyourturn: "It's not your turn",
                celltaken: "That cell is already taken",
                invalidplacement: "Invalid move",
                none: ""
            };
            const msg = errorMessages[result.error] || "Unable to make move";
            if (msg) showMessage(msg, "error");
        }
    }
}
