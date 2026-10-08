#include "status_task.h"
#include "led_strip.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "module.h"
#include "status_queue.h"

#define STATUS_TASK_TAG         "STATUS"
#define STATUS_TASK_PERIOD_MS   1000

#define LED_GPIO                8

#define CLR_LVL_LIMIT           25

// ========== LED SETTINGS
const uint8_t led_clr_lvls[MODULE_STATUS_MAX][3] =
{
    [MODULE_STATUS_OK]      = { 0, CLR_LVL_LIMIT, 0},
    [MODULE_STATUS_CONF]    = { CLR_LVL_LIMIT, CLR_LVL_LIMIT, 0},
    [MODULE_STATUS_ERROR]   = { CLR_LVL_LIMIT, 0, 0},
};
// ==========

void status_task(void *pvParameter)
{
    module_status_e module_status[MODULE_MAX] = {0};
    module_status_e max_status = MODULE_STATUS_CONF;

    for (size_t i = 0; i < MODULE_MAX; ++i)
    {
        module_status[i] = MODULE_STATUS_CONF;
    }

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
    
    
    uint8_t led_state = 0;
    
    led_strip_clear(led_strip);

    for (;;)
    {
        if (led_state) 
        {
            module_status_e incoming_status = 0;
            const bool is_any = status_queue_receive(&incoming_status, 0);

            if (is_any) 
            {
                if (incoming_status > max_status)
                {
                    max_status = incoming_status;
                }
            }
            
            led_strip_set_pixel(led_strip, 0, led_clr_lvls[max_status][0], led_clr_lvls[max_status][1], led_clr_lvls[max_status][2]);
            led_strip_refresh(led_strip);
        } 
        else 
        {
            led_strip_clear(led_strip);
        }
        
        led_state = !led_state;
        
        vTaskDelay(pdMS_TO_TICKS(STATUS_TASK_PERIOD_MS));
    }
}