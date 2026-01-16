add_executable(BattleshipWebServer
	main.cpp
 "WsGameController.cpp" "BattleshipWebSocketManager" )

find_package(Drogon CONFIG REQUIRED)

target_link_libraries(BattleshipWebServer PRIVATE
	BattleshipSessionManager
	BattleshipMessaging
	Drogon::Drogon
)

set_property(TARGET BattleshipServer PROPERTY CXX_STANDARD 20)