add_library(TicTacToeJsonProtocol
    TicTacToeJsonProtocol.cpp
    TicTacToeJsonProtocol.h
)

target_include_directories(TicTacToeJsonProtocol PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(TicTacToeJsonProtocol PUBLIC
    TicTacToeSession
    RemoteTerminalMessaging
    jsoncpp_static
)
