#include "system_events.h"
#include "esp_log.h"

static EventGroupHandle_t system_event_group = NULL;

/**
 * @brief Helper function to format and log the lower 9 bits of a number in binary format.
 * @param num Number to convert and log.
 */
static void print_binary(uint32_t num)
{
    char binary[33];

    for (int i = 8; i >= 0; i--) {
        binary[31 - i] = ((num >> i) & 1) ? '1' : '0';
    }
    binary[32] = '\0';

    ESP_LOGI("SYSTEM_EVENTS", "Binary: %s", binary);
}

/**
 * @brief Initializes the system event group.
 * @return True if event group was created successfully, false otherwise.
 * @retval true Event group initialized successfully.
 * @retval false Initialization failed.
 */
bool system_events_init(void)
{
    if (system_event_group == NULL) 
    {
        system_event_group = xEventGroupCreate();
    }

    return system_event_group != NULL;
}

/**
 * @brief Waits for specified event bits to be set within a given timeout.
 * @param bits_to_wait_for Bitmask of event bits to wait for.
 * @param delay_ms Maximum wait time in milliseconds (or portMAX_DELAY).
 * @return True if all requested bits were set before timeout, false otherwise.
 * @retval true All bits were set.
 * @retval false Timeout occurred or event group uninitialized.
 */
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

/**
 * @brief Sets specific bits in the system event group.
 * @param bits_to_set Bitmask of event bits to set.
 */
void system_events_set_bits(EventBits_t bits_to_set)
{
    if (system_event_group != NULL) 
    {
        xEventGroupSetBits(system_event_group, bits_to_set);
    }
}

/**
 * @brief Clears specific bits in the system event group.
 * @param bits_to_clear Bitmask of event bits to clear.
 */
void system_events_clear_bits(EventBits_t bits_to_clear)
{
    if (system_event_group != NULL) 
    {
        xEventGroupClearBits(system_event_group, bits_to_clear);
    }
}
