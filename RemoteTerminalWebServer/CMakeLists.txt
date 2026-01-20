add_executable(RemoteTerminalWebServer
	main.cpp
 "WsGameController.cpp" "RemoteTerminalWebSocketManager.cpp" )

find_package(Drogon CONFIG REQUIRED)

target_link_libraries(RemoteTerminalWebServer PRIVATE
	BattleshipSessionManager
	RemoteTerminalMessaging
	Drogon::Drogon
)

set_property(TARGET RemoteTerminalWebServer PROPERTY CXX_STANDARD 20)
