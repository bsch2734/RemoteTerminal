add_library(BattleshipJsonProtocol
	BattleshipJsonProtocol.cpp
	BattleshipJsonProtocol.h
)

target_include_directories(BattleshipJsonProtocol PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(BattleshipJsonProtocol PUBLIC
	BattleshipSession
    jsoncpp_static
)

set_property(TARGET BattleshipEngine PROPERTY CXX_STANDARD 20)