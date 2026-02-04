add_executable(RemoteTerminalWebServer
	main.cpp
 "RemoteTerminalWebSocketController.cpp" "RemoteTerminalWebSocketManager.cpp" "GamesHttpController.cpp" )

find_package(Drogon CONFIG REQUIRED)

target_link_libraries(RemoteTerminalWebServer PRIVATE
	BattleshipEndpoint
	TicTacToeEndpoint
	RemoteTerminalMessaging
	Drogon::Drogon
)

set_property(TARGET RemoteTerminalWebServer PROPERTY CXX_STANDARD 20)
