idf_component_register(SRCS 
                    "net_queue.c" 
                    "status_queue.c" 
                    "zb_state_queue.c" 
                    "system_events.c" 
                    "dev_state.c"
                    INCLUDE_DIRS "include")

include(FetchContent)

set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    zanni-protocol
    GIT_REPOSITORY https://github.com/Ggorets0dev/zanni-gate-protocol.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(zanni-protocol)

target_link_libraries(${COMPONENT_LIB} PUBLIC zanni-protocol)