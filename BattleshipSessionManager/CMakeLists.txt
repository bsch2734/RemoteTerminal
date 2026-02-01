add_library(BattleshipSessionManager
	BattleshipSessionManager.cpp
	BattleshipSessionManager.h
)

target_include_directories(BattleshipSessionManager PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(BattleshipSessionManager PUBLIC
		BattleshipSession
		RemoteTerminalMessaging
	PRIVATE
		BattleshipJsonProtocol
)

set_property(TARGET BattleshipSessionManager PROPERTY CXX_STANDARD 20)
