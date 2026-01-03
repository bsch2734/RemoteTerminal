add_executable(BattleshipServer
    "main.cpp"
    "ServerProtocol.h"
    "ServerProtocol.cpp"
)

target_link_libraries(BattleshipServer PRIVATE
    BattleshipSession
    jsoncpp_static
)

set_property(TARGET BattleshipServer PROPERTY CXX_STANDARD 20)