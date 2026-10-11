#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_random.h"

#include "status_task.h"
#include "status_queue.h"
#include "net_queue.h"
#include "system_events.h"
#include "zigbee_init.h"
#include "zigbee_task.h"
#include "wifi_task.h"
#include "wifi_init.h"

static const char *TAG = "main";

static void zanni_nvs_flash_init(void)
{
    esp_err_t err = nvs_flash_init();
    
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        nvs_flash_erase();
        nvs_flash_init();
    }
}

void app_main(void)
{
    bool is_inited = true;

    ESP_LOGI(TAG, "Starting FreeRTOS test tasks...");

    zanni_nvs_flash_init();

    is_inited &= status_queue_init();
    is_inited &= net_queue_init();
    is_inited &= system_events_init();
    
    if (!is_inited)
    {
        ESP_LOGE(TAG, "Failed to init some FreeRTOS objects");
        esp_restart();
    }

    xTaskCreate(&status_task, "status_task", 2048, NULL, 4, NULL);
    xTaskCreate(&zigbee_task, "zigbee_task", 5120, NULL, 5, NULL);
    xTaskCreate(&wifi_task, "wifi_task", 5120, NULL, 6, NULL);
}
