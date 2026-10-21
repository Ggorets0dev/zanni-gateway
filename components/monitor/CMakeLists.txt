if(IS_MONITOR_TASK_ENABLED)
    add_compile_definitions(IS_MONITOR_TASK_ENABLED=1)
endif()

idf_component_register(SRCS 
                    "monitor_task.c" 
                    INCLUDE_DIRS "include"
                    REQUIRES esp_timer common)