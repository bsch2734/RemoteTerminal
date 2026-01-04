// Grab references to HTML elements by id
const userIdInput = document.getElementById("userid");
const gameIdInput = document.getElementById("gameid");
const connectBtn = document.getElementById("connectBtn");
const statusSpan = document.getElementById("status");
const logPre = document.getElementById("log");

// Helper: append a line to the log
function logLine(text) {
  logPre.textContent += text + "\n";
}

// WebSocket reference (null until connected)
let socket = null;

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

