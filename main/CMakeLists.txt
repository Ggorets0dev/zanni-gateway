set(component_requires nvs_flash wifi zigbee status common monitor)

if(IS_MONITOR_TASK_ENABLED)
    message(STATUS "Monitor task is enabled for build")
    add_compile_definitions(IS_MONITOR_TASK_ENABLED=1)
endif()

idf_component_register(SRCS "main.c" 
                    INCLUDE_DIRS "." 
                    REQUIRES ${component_requires})