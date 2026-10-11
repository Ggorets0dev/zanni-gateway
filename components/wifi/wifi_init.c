#include "wifi_init.h"
#include <string.h>
#include <stdbool.h>

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "system_events.h"
#include "module.h"

#define CONNECT_RETRY_MAX   10

#ifndef WIFI_SSID
#error "WIFI SSID is not specified during build"
#endif

#ifndef WIFI_PASS
#error "WIFI PASS is not specified during build"
#endif

#define TAG     g_module_tag[MODULE_WIFI]

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data) 
{
    (void)arg;
    static int s_retry_num = 0;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        system_events_clear_bits(SYS_EVENT_WIFI_CONNECTED_BIT);
        wifi_event_sta_disconnected_t* event = (wifi_event_sta_disconnected_t*) event_data;
        ESP_LOGW(TAG, "Disconnected from WIFI! Reason code: %d", event->reason);

        if (s_retry_num < CONNECT_RETRY_MAX) 
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Reconnecting to WIFI spot (attempt %d)", s_retry_num);
            ESP_LOGI(TAG, "WIFI SSID: [%s], PASS: [%s]", WIFI_SSID, WIFI_PASS);
        }
        else
        {
            ESP_LOGE(TAG, "Failed to connect to WIFI spot");
            system_events_set_bits(SYS_EVENT_GLOBAL_FAULT_BIT);
        }
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;

        system_events_set_bits(SYS_EVENT_WIFI_CONNECTED_BIT);
    }
}

void zanni_wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
                                                        
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = 
    {
        .sta = 
        {
            .pmf_cfg = 
            {
                .capable = true,
                .required = false
            },
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    strncpy((char *)wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, WIFI_PASS, sizeof(wifi_config.sta.password) - 1);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}