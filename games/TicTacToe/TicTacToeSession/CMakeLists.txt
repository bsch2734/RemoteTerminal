add_library(TicTacToeSession
	TicTacToeSession.cpp
	TicTacToeSession.h
	SessionTypes.h
)

target_include_directories(TicTacToeSession PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(TicTacToeSession PUBLIC
	TicTacToeEngine
	RemoteTerminalCore
)
