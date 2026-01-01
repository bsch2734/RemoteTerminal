add_executable(BattleshipServer
    "main.cpp"
 "BattleshipSession.cpp" "Action.h" "ServerProtocol.h" "ServerProtocol.cpp" "SessionTypes.h")

target_link_libraries(BattleshipServer PRIVATE
    BattleshipEngine
    jsoncpp_static
)

set_property(TARGET BattleshipServer PROPERTY CXX_STANDARD 20)