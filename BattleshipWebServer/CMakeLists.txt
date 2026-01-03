add_executable(BattleshipWebServer
	main.cpp
)

find_package(Drogon CONFIG REQUIRED)

target_link_libraries(BattleshipWebServer PRIVATE
	Drogon::Drogon
)
