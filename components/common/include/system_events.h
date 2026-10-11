#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define SYS_EVENT_WIFI_CONNECTED_BIT    BIT0
#define SYS_EVENT_NODES_CONNECTED_BIT   BIT1
#define SYS_EVENT_ZIGBEE_CONNECTED_BIT  BIT2
#define SYS_EVENT_GLOBAL_FAULT_BIT      BIT3

#define SYS_EVENT_SYSTEM_ONLINE         (SYS_EVENT_WIFI_CONNECTED_BIT | SYS_EVENT_NODES_CONNECTED_BIT | SYS_EVENT_ZIGBEE_CONNECTED_BIT)

bool system_events_init(void);

// Wait for specific system events. Returns pdTRUE if all bits were set before timeout.
bool system_events_wait_bits(EventBits_t bits_to_wait_for, uint32_t delay_ms);

// Set specific system events
void system_events_set_bits(EventBits_t bits_to_set);

// Clear specific system events
void system_events_clear_bits(EventBits_t bits_to_clear);
