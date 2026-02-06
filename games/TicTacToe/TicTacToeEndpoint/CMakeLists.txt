add_library(TicTacToeEndpoint
    TicTacToeEndpoint.cpp
)

target_include_directories(TicTacToeEndpoint PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(TicTacToeEndpoint
    PUBLIC
        RemoteTerminalMessaging
        TicTacToeSessionManager
    PRIVATE
        TicTacToeJsonProtocol
)
