add_library(BattleshipEndpoint
	BattleshipEndpoint.cpp
)

target_include_directories(BattleshipEndpoint PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(BattleshipEndpoint
	PUBLIC
		RemoteTerminalMessaging
		BattleshipSessionManager
	PRIVATE
		BattleshipJsonProtocol
)
