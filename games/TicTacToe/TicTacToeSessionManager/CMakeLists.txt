add_library(TicTacToeSessionManager
    TicTacToeSessionManager.cpp
)

target_include_directories(TicTacToeSessionManager PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(TicTacToeSessionManager
    PUBLIC
        RemoteTerminalMessaging
        TicTacToeSession
)
