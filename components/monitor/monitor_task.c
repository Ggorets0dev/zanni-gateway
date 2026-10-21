#ifdef IS_MONITOR_TASK_ENABLED

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "esp_clk_tree.h"
#include "soc/clk_tree_defs.h"
#include "module.h"

#define TAG                         g_module_config[MODULE_MONITOR].tag
#define MONITOR_TASK_PERIOD_MS      5000

void monitor_task(void *pvParameters)
{
    uint32_t cpu_freq_hz = 0;

    for (;;) 
    {
        esp_clk_tree_src_get_freq_hz(SOC_MOD_CLK_CPU, 
                                     ESP_CLK_TREE_SRC_FREQ_PRECISION_CACHED, 
                                     &cpu_freq_hz);

        uint32_t free_heap = esp_get_free_heap_size();
        uint32_t min_free_heap = esp_get_minimum_free_heap_size();
        size_t largest_block = heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);

        int64_t uptime_sec = esp_timer_get_time() / 1000000;

        ESP_LOGI(TAG, "=== System Status (Uptime: %lld s) ===", uptime_sec);
        ESP_LOGI(TAG, "CPU Freq: %lu MHz", cpu_freq_hz / 1000000);
        ESP_LOGI(TAG, "Heap: Free=%lu B | Min=%lu B | MaxBlock=%u B", 
                 free_heap, min_free_heap, largest_block);

#if CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
        char *stats_buf = malloc(1024);
        if (stats_buf) {
            vTaskGetRunTimeStats(stats_buf);
            printf("\nTask Name\tAbs Time\t%% Time\n");
            printf("----------------------------------------\n%s\n", stats_buf);
            free(stats_buf);
        }
#endif

        vTaskDelay(pdMS_TO_TICKS(MONITOR_TASK_PERIOD_MS));
    }
}

#endif // IS_MONITOR_TASK_ENABLED