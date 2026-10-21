#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_pm.h"
#include "nvs_flash.h"
#include "esp_random.h"

#include "status_task.h"
#include "status_queue.h"
#include "net_queue.h"
#include "zb_state_queue.h"
#include "system_events.h"
#include "module.h"
#include "zigbee_zboss_task.h"
#include "zigbee_state_task.h"

#ifdef IS_MONITOR_TASK_ENABLED
#include "monitor_task.h"
#endif

#include "wifi_task.h"
#include "wifi_init.h"

static const char *TAG = "main";

/**
 * @brief Initializes NVS (Non-Volatile Storage) flash, erasing and re-initializing if storage is corrupted or full.
 */
static void zanni_nvs_flash_init(void)
{
    esp_err_t err = nvs_flash_init();
    
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        nvs_flash_erase();
        nvs_flash_init();
    }
}

/**
 * @brief Configures global dynamic frequency scaling and power management parameters.
 */
static void zanni_pm_setup(void) 
{
    esp_pm_config_t pm_config = 
    {
        .max_freq_mhz = 80,
        .min_freq_mhz = 40,
        .light_sleep_enable = false
    };
    ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
    ESP_LOGI(TAG, "Global PM configuration is completed");
}

/**
 * @brief Main application entry point initializing power management, NVS, FreeRTOS queues, and launching system tasks.
 */
void app_main(void)
{
    bool is_inited = true;

    zanni_pm_setup();
    zanni_nvs_flash_init();

    is_inited &= status_queue_init();
    is_inited &= net_queue_init();
    is_inited &= zb_state_queue_init();
    is_inited &= system_events_init();
    
    if (!is_inited)
    {
        ESP_LOGE(TAG, "Failed to init some FreeRTOS objects");
        esp_restart();
    }

    ESP_LOGI(TAG, "Starting FreeRTOS tasks...");

    MODULE_START(MODULE_STATUS, status_task, NULL);
    MODULE_START(MODULE_ZIGBEE_ZBOSS, zigbee_zboss_task, NULL);
    MODULE_START(MODULE_ZIGBEE_STATE, zigbee_state_task, NULL);
    MODULE_START(MODULE_WIFI, wifi_task, NULL);

#ifdef IS_MONITOR_TASK_ENABLED
    MODULE_START(MODULE_MONITOR, monitor_task, NULL);
#endif
}
