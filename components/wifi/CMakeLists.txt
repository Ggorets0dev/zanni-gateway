if(WIFI_SSID)
    add_compile_definitions(WIFI_SSID=\"${WIFI_SSID}\")
endif()

if(WIFI_PASS)
    add_compile_definitions(WIFI_PASS=\"${WIFI_PASS}\")
endif()

idf_component_register(SRCS "wifi_init.c"
                        INCLUDE_DIRS "include"
                        REQUIRES esp_wifi esp_netif lwip esp_event common)