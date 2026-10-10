#include "system_events.h"
#include "esp_log.h"

static EventGroupHandle_t system_event_group = NULL;

bool system_events_init(void)
{
    if (system_event_group == NULL) 
    {
        system_event_group = xEventGroupCreate();
    }

    return system_event_group != NULL;
}

BaseType_t system_events_wait_bits(EventBits_t bits_to_wait_for, TickType_t xTicksToWait)
{
    if (system_event_group == NULL) 
    {
        return pdFALSE;
    }
    EventBits_t bits = xEventGroupWaitBits(system_event_group, bits_to_wait_for, pdFALSE, pdTRUE, xTicksToWait);
    return ((bits & bits_to_wait_for) == bits_to_wait_for) ? pdTRUE : pdFALSE;
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
