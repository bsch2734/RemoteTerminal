add_executable(BattleshipServer
    "main.cpp"
)

target_link_libraries(BattleshipServer PRIVATE
    BattleshipEndpoint
    BattleshipJsonProtocol
)

set_property(TARGET BattleshipServer PROPERTY CXX_STANDARD 20)