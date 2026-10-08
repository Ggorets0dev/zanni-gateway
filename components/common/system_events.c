#include "system_events.h"
#include "esp_log.h"

static EventGroupHandle_t s_system_event_group = NULL;

void system_events_init(void)
{
    if (s_system_event_group == NULL) {
        s_system_event_group = xEventGroupCreate();
    }
}

BaseType_t system_events_wait_bits(EventBits_t bits_to_wait_for, TickType_t xTicksToWait)
{
    if (s_system_event_group == NULL) {
        return pdFALSE;
    }
    EventBits_t bits = xEventGroupWaitBits(s_system_event_group, bits_to_wait_for, pdFALSE, pdTRUE, xTicksToWait);
    return ((bits & bits_to_wait_for) == bits_to_wait_for) ? pdTRUE : pdFALSE;
}

void system_events_set_bits(EventBits_t bits_to_set)
{
    if (s_system_event_group != NULL) {
        xEventGroupSetBits(s_system_event_group, bits_to_set);
    }
}

void system_events_clear_bits(EventBits_t bits_to_clear)
{
    if (s_system_event_group != NULL) {
        xEventGroupClearBits(s_system_event_group, bits_to_clear);
    }
}
