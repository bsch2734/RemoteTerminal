add_executable(BattleshipWebServer
	main.cpp
 "WsGameController.cpp")

find_package(Drogon CONFIG REQUIRED)

target_link_libraries(BattleshipWebServer PRIVATE
	BattleshipJsonProtocol
	Drogon::Drogon
)
