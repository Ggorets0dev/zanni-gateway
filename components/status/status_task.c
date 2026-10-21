#include "led_strip.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "module.h"
#include "status_task.h"
#include "status_queue.h"
#include "system_events.h"

/**
 * @brief Logging tag for the status LED task.
 */
#define TAG                             g_module_config[MODULE_STATUS].tag

/**
 * @brief Blinking period in milliseconds for persistent global statuses.
 */
#define STATUS_TASK_GLOBAL_PERIOD_MS    800

/**
 * @brief Fast blinking period in milliseconds for transient communication events.
 */
#define STATUS_TASK_EVENT_PERIOD_MS     100

/**
 * @brief GPIO pin number connected to the WS2812 addressable RGB LED data line.
 */
#define LED_GPIO                        8

/**
 * @brief Maximum intensity limit for RGB LED color channels to prevent blinding brightness.
 */
#define CLR_LVL_LIMIT                   15

// ========== LED SETTINGS
const uint8_t led_clr_lvls[MODULE_STATUS_MAX][3] =
{
    [MODULE_STATUS_OK]      = { 0, CLR_LVL_LIMIT, 0},
    [MODULE_STATUS_CONF]    = { CLR_LVL_LIMIT, CLR_LVL_LIMIT, 0},
    [MODULE_STATUS_TX]      = { CLR_LVL_LIMIT, 0, CLR_LVL_LIMIT},
    [MODULE_STATUS_RX]      = { 0, 0, CLR_LVL_LIMIT},
    [MODULE_STATUS_ERROR]   = { CLR_LVL_LIMIT, 0, 0},
};
// ==========

/**
 * @brief Checks whether a given status code corresponds to a transient communication event.
 * @param status Module status to evaluate.
 * @return True if status represents RX or TX event, false otherwise.
 * @retval true Status is MODULE_STATUS_RX or MODULE_STATUS_TX.
 * @retval false Status is another module status.
 */
static bool is_status_event(module_status_e status)
{
    if (status == MODULE_STATUS_RX || status == MODULE_STATUS_TX)
    {
        return true;
    }
    
    return false;
}

/**
 * @brief FreeRTOS task function for managing the RGB status LED indication.
 * @param pvParameter Pointer to task parameters (unused).
 */
void status_task(void *pvParameter)
{
    module_status_e curr_status = MODULE_STATUS_CONF;
    led_strip_handle_t led_strip;
    
    // LED strip general initialization
    led_strip_config_t strip_config = 
    {
        .strip_gpio_num = LED_GPIO,
        .max_leds = 1, // at least one LED on board
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .flags = 
        {
            .invert_out = false,
        }
    };

    // LED strip backend configuration: RMT
    led_strip_rmt_config_t rmt_config = 
    {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    
    bool led_state = 0;
    
    led_strip_clear(led_strip);

    for (;;)
    {
        if (led_state) 
        {
            static status_msg_t incoming_status;
            const bool is_any = status_queue_receive(&incoming_status, 0);

            if (is_any) 
            {
                // Get status from queue
                curr_status = incoming_status.status;
            }
            else
            {
                // Get status from flags

                // Try to find fault
                const bool is_fault = system_events_wait_bits(SYS_EVENT_GLOBAL_FAULT_BIT, 0);
                curr_status = is_fault ? MODULE_STATUS_ERROR : MODULE_STATUS_CONF;
                
                // Try to find online
                if (!is_fault)
                {
                    const bool is_online = system_events_wait_bits(SYS_EVENT_SYSTEM_ONLINE, 0);
                    curr_status = is_online ? MODULE_STATUS_OK : MODULE_STATUS_CONF;
                }
            }
            
            led_strip_set_pixel(led_strip, 0, led_clr_lvls[curr_status][0], led_clr_lvls[curr_status][1], led_clr_lvls[curr_status][2]);
            led_strip_refresh(led_strip);
        } 
        else 
        {
            led_strip_clear(led_strip);
        }
        
        led_state = !led_state;
        
        const uint32_t delay_ms = is_status_event(curr_status) ? STATUS_TASK_EVENT_PERIOD_MS : STATUS_TASK_GLOBAL_PERIOD_MS;
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}