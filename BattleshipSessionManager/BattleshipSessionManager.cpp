#include "BattleshipSessionManager.h"

AddUserToGameResult BattleshipSessionManager::addUserToGame(UserId u, GameId g) {
	AddUserToGameResult answer;

	//game is already in progress
	auto inProgressGame = _gameIdToSessionMap.find(g);
	if (inProgressGame != _gameIdToSessionMap.end()) {
		answer.success = false;
		answer.error = AddUserToGameError::gameFull;
		answer.readyToStart = false;
		return answer;
	}

	auto lobbyGame = _lobbyGames.find(g);
	//game does not exist yet
	if (lobbyGame == _lobbyGames.end()) {
		_lobbyGames.insert({ g, u });
		answer.readyToStart = false;
		answer.success = true;
		return answer;
	}

	//game exists only in lobby
	//...and this user is trying to join twice
	if (lobbyGame->second == u) {
		answer.success = false;
		answer.error = AddUserToGameError::userAlreadyInGame;
		return answer;
	}

	//...and this is the second user
	BattleshipSession* s = new BattleshipSession(g, lobbyGame->second, u);
	_gameIdToSessionMap[g] = s;
	_lobbyGames.erase(lobbyGame);
	
	answer.success = true;
	answer.readyToStart = true;
	
	return answer;
}

void BattleshipSessionManager::destroySession(GameId g) {
	delete _gameIdToSessionMap[g];
	_gameIdToSessionMap.erase(g);
}

BattleshipSession* BattleshipSessionManager::findSession(GameId g) {
	auto f = _gameIdToSessionMap.find(g);
	if (f == _gameIdToSessionMap.end())
		return nullptr;
	return (*f).second;
}
