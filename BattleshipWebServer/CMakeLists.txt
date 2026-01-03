add_executable(BattleshipWebServer
	main.cpp
 "WsGameController.cpp" "BattleshipWebSocketManager" )

find_package(Drogon CONFIG REQUIRED)

target_link_libraries(BattleshipWebServer PRIVATE
	BattleshipJsonProtocol
	BattleshipSessionManager
	Drogon::Drogon
)
