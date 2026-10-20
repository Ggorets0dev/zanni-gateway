idf_component_register(SRCS 
                        "zigbee_state_task.c" 
                        "zigbee_zboss_task.c" 
                        "zigbee_init.c" 
                        "zigbee_transport.c"
                        INCLUDE_DIRS "include"
                        REQUIRES espressif__esp-zigbee-lib common ieee802154 esp_hw_support esp_timer)

