import "./style.css";

// === Configuration ===
const API_URL = import.meta.env.VITE_API_URL || "";

// === DOM References ===
const gamesLoading = document.getElementById("gamesLoading");
const gamesError = document.getElementById("gamesError");
const gamesList = document.getElementById("gamesList");
const retryBtn = document.getElementById("retryBtn");
const serverUrlSpan = document.getElementById("serverUrl");

// === State ===
let availableGames = [];

// === Initialize ===
serverUrlSpan.textContent = API_URL;
loadGames();

// === Event Listeners ===
retryBtn.addEventListener("click", loadGames);

// === Game Loading ===
async function loadGames() {
    gamesLoading.classList.remove("hidden");
    gamesError.classList.add("hidden");
    gamesList.classList.add("hidden");

    try {
        const response = await fetch(`${API_URL}/api/games`);
        if (!response.ok) throw new Error(`HTTP ${response.status}`);
        
        const data = await response.json();
        availableGames = data.games || [];
        
        renderGamesList();
        
        gamesLoading.classList.add("hidden");
        gamesList.classList.remove("hidden");
    } catch (error) {
        console.error("Error loading games:", error);
        gamesLoading.classList.add("hidden");
        gamesError.classList.remove("hidden");
        document.querySelector(".error-text").textContent = `Failed to load games: ${error.message}`;
    }
}

function renderGamesList() {
    gamesList.innerHTML = "";
    
    if (availableGames.length === 0) {
        gamesList.innerHTML = `
            <div class="no-games">
                <span>No games available</span>
            </div>
        `;
        return;
    }
    
    for (const game of availableGames) {
        const card = document.createElement("a");
        card.className = "game-card";
        card.href = game.url || `/${game.id}/`;
        
        card.innerHTML = `
            <div class="game-card-icon">${game.icon || "&#127918;"}</div>
            <div class="game-card-content">
                <h2 class="game-card-title">${escapeHtml(game.name)}</h2>
                <p class="game-card-description">${escapeHtml(game.description)}</p>
                <div class="game-card-meta">
                    <span class="player-count">${game.minPlayers}-${game.maxPlayers} players</span>
                </div>
            </div>
            <div class="game-card-arrow">&#8594;</div>
        `;
        
        gamesList.appendChild(card);
    }
}

function escapeHtml(text) {
    const div = document.createElement("div");
    div.textContent = text;
    return div.innerHTML;
}

