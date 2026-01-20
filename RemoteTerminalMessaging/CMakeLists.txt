add_library(RemoteTerminalMessaging
	RemoteTerminalMessageRouter.cpp
	RemoteTerminalMessageRouter.h
)

target_include_directories(RemoteTerminalMessaging PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(RemoteTerminalMessaging PUBLIC
	BattleshipSessionManager
)

set_property(TARGET RemoteTerminalMessaging PROPERTY CXX_STANDARD 20)
