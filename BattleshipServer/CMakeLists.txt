add_executable(BattleshipServer
    "main.cpp"
 "BattleshipSession.cpp" "Action.h")

target_link_libraries(BattleshipServer PRIVATE
    BattleshipEngine
)

set_property(TARGET BattleshipServer PROPERTY CXX_STANDARD 20)