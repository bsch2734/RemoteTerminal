# RemoteTerminalCore - foundational types for the RemoteTerminal system
# This is a header-only library containing domain primitives like UserId, GameId,
# and message addressing types that other modules depend on.

add_library(RemoteTerminalCore INTERFACE)

target_include_directories(RemoteTerminalCore INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}
)
