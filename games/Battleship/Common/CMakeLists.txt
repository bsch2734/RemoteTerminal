add_library(NavalBattleCommon
	coord.cpp
	coord.h
	Fleet.cpp
	Fleet.h
	GameEntities.h
	Ship.cpp
	Ship.h
)

target_include_directories(NavalBattleCommon PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

set_property(TARGET NavalBattleCommon PROPERTY CXX_STANDARD 20)
