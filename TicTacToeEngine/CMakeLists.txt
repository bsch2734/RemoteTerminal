
add_library(TicTacToeEngine STATIC
    "TicTacToeEngine.cpp"
 "TicTacToeEntities.h" )

target_include_directories(TicTacToeEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set_property(TARGET TicTacToeEngine PROPERTY CXX_STANDARD 20)
