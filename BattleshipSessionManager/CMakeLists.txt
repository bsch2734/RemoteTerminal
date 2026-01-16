add_library(BattleshipSessionManager
	BattleshipSessionManager.cpp
	BattleshipSessionManager.h
)

target_include_directories(BattleshipSessionManager PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(BattleshipSessionManager PUBLIC
	BattleshipSession
	BattleshipJsonProtocol
)

set_property(TARGET BattleshipEngine PROPERTY CXX_STANDARD 20)
