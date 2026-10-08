#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_random.h"

#include "common.h"
#include "status_task.h"
#include "status_queue.h"
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
    ESP_LOGI(TAG, "Starting FreeRTOS test tasks...");

    zanni_nvs_flash_init();

    status_queue_init();
    system_events_init();
    
    // Create Blink Task
    xTaskCreate(&status_task, "status_task", 2048, NULL, 4, NULL);
    xTaskCreate(&zigbee_task, "zigbee_task", 4096, NULL, 5, NULL);
    xTaskCreate(&wifi_task, "wifi_task", 4096, NULL, 5, NULL);
}
