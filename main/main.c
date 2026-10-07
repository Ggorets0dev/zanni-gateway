#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_random.h"

#include "led_task.h"
#include "zigbee_init.h"
#include "wifi_init.h"

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

    zanni_zigbee_init();

    // Create Task 1
    xTaskCreate(&task1, "task1", 2048, NULL, 5, NULL);
    
    // Create Task 2
    xTaskCreate(&task2, "task2", 2048, NULL, 5, NULL);
    
    // Create Blink Task
    xTaskCreate(&led_task, "led_task", 2048, NULL, 5, NULL);
}
