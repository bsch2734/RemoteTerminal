add_library(RemoteTerminalMessaging
	"RemoteTerminalEndpointRegistry.cpp"
 "IRemoteTerminalEndpoint.h" "EndpointTypes.h")

target_include_directories(RemoteTerminalMessaging PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(RemoteTerminalMessaging PUBLIC
	RemoteTerminalCore
)

set_property(TARGET RemoteTerminalMessaging PROPERTY CXX_STANDARD 20)
