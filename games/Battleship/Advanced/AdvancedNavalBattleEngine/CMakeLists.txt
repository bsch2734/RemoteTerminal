add_library(AdvancedNavalBattleEngine STATIC
	AdvancedNavalBattleEngine.cpp
	AdvancedNavalBattleEngine.h
	AdvancedFleet.cpp
	AdvancedFleet.h
	AdvancedGameEntities.h
)

target_link_libraries(AdvancedNavalBattleEngine PUBLIC 
	NavalBattleCommon
	AdvancedShip
)

target_include_directories(AdvancedNavalBattleEngine PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

set_property(TARGET AdvancedNavalBattleEngine PROPERTY CXX_STANDARD 20)
