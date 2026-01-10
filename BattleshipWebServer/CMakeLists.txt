add_executable(BattleshipWebServer
	main.cpp
 "WsGameController.cpp" "BattleshipWebSocketManager"  "BattleshipMessageRouter.cpp")

find_package(Drogon CONFIG REQUIRED)

target_link_libraries(BattleshipWebServer PRIVATE
	BattleshipJsonProtocol
	BattleshipSessionManager
	Drogon::Drogon
)

set_property(TARGET BattleshipServer PROPERTY CXX_STANDARD 20)