#include "system_events.h"
#include "esp_log.h"

static EventGroupHandle_t system_event_group = NULL;

static void print_binary(uint32_t num)
{
    char binary[33];

    for (int i = 8; i >= 0; i--) {
        binary[31 - i] = ((num >> i) & 1) ? '1' : '0';
    }
    binary[32] = '\0';

    ESP_LOGI("SYSTEM_EVENTS", "Binary: %s", binary);
}

bool system_events_init(void)
{
    if (system_event_group == NULL) 
    {
        system_event_group = xEventGroupCreate();
    }

    return system_event_group != NULL;
}

bool system_events_wait_bits(EventBits_t bits_to_wait_for, uint32_t delay_ms)
{
    if (system_event_group == NULL) 
    {
        return pdFALSE;
    }
    EventBits_t bits = xEventGroupWaitBits(system_event_group, bits_to_wait_for, pdFALSE, pdTRUE, pdMS_TO_TICKS(delay_ms));
    
    // print_binary(bits);

    return ((bits & bits_to_wait_for) == bits_to_wait_for);
}

void system_events_set_bits(EventBits_t bits_to_set)
{
    if (system_event_group != NULL) 
    {
        xEventGroupSetBits(system_event_group, bits_to_set);
    }
}

void system_events_clear_bits(EventBits_t bits_to_clear)
{
    if (system_event_group != NULL) 
    {
        xEventGroupClearBits(system_event_group, bits_to_clear);
    }
}
