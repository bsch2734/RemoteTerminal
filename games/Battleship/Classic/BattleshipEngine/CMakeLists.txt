# CMakeList.txt : CMake project for BattleshipEngine, include source and define
# project specific logic here.
#

add_library(BattleshipEngine STATIC
    "BattleshipEngine.cpp"
    "BattleshipEngine.h"
)

target_link_libraries(BattleshipEngine PUBLIC NavalBattleCommon)

target_include_directories(BattleshipEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set_property(TARGET BattleshipEngine PROPERTY CXX_STANDARD 20)

# TODO: Add tests and install targets if needed.
