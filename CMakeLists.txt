# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.10)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("RemoteTerminal")

# Use C++ 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# Force jsoncpp (and other libs) to build as static
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libs" FORCE)
# jsoncpp-specific switches (matches their docs: BUILD_STATIC_LIBS / BUILD_SHARED_LIBS)
set(BUILD_STATIC_LIBS ON CACHE BOOL "Build static jsoncpp libs" FORCE)

# Disable jsoncpp tests and post-build test runs
set(JSONCPP_WITH_TESTS OFF CACHE BOOL "Disable jsoncpp tests" FORCE)
set(JSONCPP_WITH_POST_BUILD_UNITTEST OFF CACHE BOOL "Disable jsoncpp post build unit tests" FORCE)

# Include sub-projects.
add_subdirectory(third_party/jsoncpp)
# force jsoncpp to use C++20
set_property(TARGET jsoncpp_static PROPERTY CXX_STANDARD 20)
set_property(TARGET jsoncpp_static PROPERTY CXX_STANDARD_REQUIRED ON)

add_subdirectory ("RemoteTerminalCore")
add_subdirectory ("BattleshipEngine")
add_subdirectory ("BattleshipSession")
add_subdirectory ("BattleshipSessionManager")
add_subdirectory ("RemoteTerminalMessaging")
add_subdirectory ("BattleshipJsonProtocol")
add_subdirectory ("BattleshipServer")
add_subdirectory ("RemoteTerminalWebServer")
add_subdirectory ("DemoGame")
add_subdirectory ("BattleshipEndpoint")
add_subdirectory ("TicTacToeEngine")
add_subdirectory ("TicTacToeSession")
add_subdirectory ("TicTacToeSessionManager")
add_subdirectory ("TicTacToeJsonProtocol")
add_subdirectory ("TicTacToeEndpoint")
add_subdirectory ("TicTacToeRepl")


install(TARGETS RemoteTerminalWebServer RUNTIME DESTINATION bin)