#pragma once
#include "BattleshipSession.h"
#include <map>

enum class AddUserToGameError {
	userAlreadyInGame,
	gameFull
};

struct AddUserToGameResult {
	bool success = false;
	bool readyToStart = false;
	AddUserToGameError error;
};

class BattleshipSessionManager {
public:
	AddUserToGameResult addUserToGame(UserId u, GameId g);
	void destroySession(GameId g);

	BattleshipSession* findSession(GameId g);

private:
	std::map<GameId, BattleshipSession*> _gameIdToSessionMap;
	std::map<GameId, UserId> _lobbyGames;
};
