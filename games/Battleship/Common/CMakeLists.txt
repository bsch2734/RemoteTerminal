add_library(NavalBattleCommon
	NavalBattleCommon.cpp
	NavalBattleCommon.h
)

target_include_directories(NavalBattleCommon PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

set_property(TARGET NavalBattleCommon PROPERTY CXX_STANDARD 20)
