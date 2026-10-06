#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_zigbee.h"
#include "nvs_flash.h"

static const char *TAG = "main";

void task1(void *pvParameter)
{
    while(1) {
        ESP_LOGI(TAG, "Task 1 is running...");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void task2(void *pvParameter)
{
    while(1) {
        ESP_LOGI(TAG, "Task 2 is running...");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting FreeRTOS test tasks...");
    
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        nvs_flash_erase();
        nvs_flash_init();
    }

    esp_zigbee_config_t config = {
        .device_config = {
            .device_type = EZB_NWK_DEVICE_TYPE_COORDINATOR,
            .install_code_policy = false,
            .zczr_config = {
                .max_children = 10
            }
        },
        .platform_config = {
            .radio_config = {
                .radio_mode = ESP_ZIGBEE_RADIO_MODE_NATIVE
            },
        }
    };
    esp_zigbee_init(&config);

    // Create Task 1
    xTaskCreate(&task1, "task1", 2048, NULL, 5, NULL);
    
    // Create Task 2
    xTaskCreate(&task2, "task2", 2048, NULL, 5, NULL);
}
