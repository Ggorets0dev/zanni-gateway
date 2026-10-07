#include "led_task.h"
#include "led_strip.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define LED_TASK_PERIOD_MS  1000

#define LED_GPIO            8

#define CLR_LVL_LIMIT       25

// KEEP ALIVE COLOR SETTINGS
#define KA_RED_LVL          25
#define KA_GREEN_LVL        25
#define KA_BLUE_LVL         0

#if KA_RED_LVL > CLR_LVL_LIMIT
#error "KA RED level is over limit"
#elif KA_GREEN_LVL > CLR_LVL_LIMIT
#error "KA GREEN level is over limit"
#elif KA_BLUE_LVL > CLR_LVL_LIMIT
#error "KA BLUE level is over limit"
#endif 

static const char *TAG = "main";

void led_task(void *pvParameter)
{
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
    
    led_strip_clear(led_strip);

    uint8_t led_state = 0;
    
    while(1) 
    {
        if (led_state) 
        {
            // ESP_LOGI(TAG, "Turning the LED ON with random color: R=%lu, G=%lu, B=%lu", KA_RED_LVL, KA_GREEN_LVL, KA_BLUE_LVL);
            ESP_LOGI(TAG, "Turning keep alive LED ON");
            led_strip_set_pixel(led_strip, 0, KA_RED_LVL, KA_GREEN_LVL, KA_BLUE_LVL);
            led_strip_refresh(led_strip);
        } 
        else 
        {
            ESP_LOGI(TAG, "Turning keep alive LED OFF");
            led_strip_clear(led_strip);
        }
        
        led_state = !led_state;
        
        vTaskDelay(pdMS_TO_TICKS(LED_TASK_PERIOD_MS));
    }
}