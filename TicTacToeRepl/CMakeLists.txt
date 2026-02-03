cmake_minimum_required(VERSION 3.10)

project(TicTacToeRepl)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(TicTacToeRepl main.cpp)

target_link_libraries(TicTacToeRepl PRIVATE TicTacToeEngine)