add_library(AdvancedShip STATIC
	AdvancedShip.cpp
	AdvancedShip.h
)

target_link_libraries(AdvancedShip PUBLIC NavalBattleCommon)

target_include_directories(AdvancedShip PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

set_property(TARGET AdvancedShip PROPERTY CXX_STANDARD 20)

add_subdirectory(AdvancedNavalBattleEngine)
