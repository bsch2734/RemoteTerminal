add_executable(DemoGame
    "main.cpp"
)

target_link_libraries(DemoGame PRIVATE
    BattleshipEngine
)
