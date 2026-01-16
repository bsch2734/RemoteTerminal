add_library(BattleshipMessaging
	BattleshipMessageRouter.cpp
	BattleshipMessageRouter.h
)

target_include_directories(BattleshipMessaging PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(BattleshipMessaging PUBLIC
	BattleshipSessionManager
)

set_property(TARGET BattleshipMessaging PROPERTY CXX_STANDARD 20)
