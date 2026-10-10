#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define SYS_EVENT_WIFI_CONNECTED_BIT BIT0

bool system_events_init(void);

// Wait for specific system events. Returns pdTRUE if all bits were set before timeout.
BaseType_t system_events_wait_bits(EventBits_t bits_to_wait_for, TickType_t xTicksToWait);

// Set specific system events
void system_events_set_bits(EventBits_t bits_to_set);

// Clear specific system events
void system_events_clear_bits(EventBits_t bits_to_clear);
