idf_component_register(SRCS 
                    "net_queue.c" 
                    "status_queue.c" 
                    "zb_state_queue.c" 
                    "system_events.c" 
                    "dev_state.c"
                    INCLUDE_DIRS "include")

if(IS_MONITOR_TASK_ENABLED)
    add_compile_definitions(IS_MONITOR_TASK_ENABLED=1)
endif()

include(FetchContent)

set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    zanni-protocol
    GIT_REPOSITORY https://github.com/Ggorets0dev/zanni-protocol.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(zanni-protocol)

target_link_libraries(${COMPONENT_LIB} PUBLIC zanni-protocol)