add_library(BattleshipSession
	BattleshipSession.cpp
	BattleshipSession.h
	Action.h
	SessionTypes.h
)

target_include_directories(BattleshipSession PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(BattleshipSession PUBLIC
	BattleshipEngine
	RemoteTerminalCore
)
