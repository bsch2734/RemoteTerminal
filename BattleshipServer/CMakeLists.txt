add_executable(BattleshipServer
    "main.cpp"
)

target_link_libraries(BattleshipServer PRIVATE
    BattleshipEngine
)
