# CMakeList.txt : CMake project for BattleshipEngine, include source and define
# project specific logic here.
#

add_library(BattleshipEngine STATIC
    "BattleshipEngine.cpp"
)

target_include_directories(BattleshipEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET BattleshipEngine PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
