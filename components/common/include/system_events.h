#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

/**
 * @brief Event bit indicating Wi-Fi connection is established.
 */
#define SYS_EVENT_WIFI_CONNECTED_BIT    BIT0

/**
 * @brief Event bit indicating TCP connection to all remote nodes is established.
 */
#define SYS_EVENT_NODES_CONNECTED_BIT   BIT1

/**
 * @brief Event bit indicating Zigbee network connection is established.
 */
#define SYS_EVENT_ZIGBEE_CONNECTED_BIT  BIT2

/**
 * @brief Event bit indicating a critical global system fault occurred.
 */
#define SYS_EVENT_GLOBAL_FAULT_BIT      BIT3

/**
 * @brief Combined bitmask representing all subsystems online (Wi-Fi, Nodes, Zigbee).
 */
#define SYS_EVENT_SYSTEM_ONLINE         (SYS_EVENT_WIFI_CONNECTED_BIT | SYS_EVENT_NODES_CONNECTED_BIT | SYS_EVENT_ZIGBEE_CONNECTED_BIT)

/**
 * @brief Initializes the system event group.
 * @return True if event group was created successfully, false otherwise.
 * @retval true Event group initialized successfully.
 * @retval false Initialization failed.
 */
bool system_events_init(void);

/**
 * @brief Waits for specified event bits to be set within a given timeout.
 * @param bits_to_wait_for Bitmask of event bits to wait for.
 * @param delay_ms Maximum wait time in milliseconds (or portMAX_DELAY).
 * @return True if all requested bits were set before timeout, false otherwise.
 * @retval true All bits were set.
 * @retval false Timeout occurred or event group uninitialized.
 */
bool system_events_wait_bits(EventBits_t bits_to_wait_for, uint32_t delay_ms);

/**
 * @brief Sets specific bits in the system event group.
 * @param bits_to_set Bitmask of event bits to set.
 */
void system_events_set_bits(EventBits_t bits_to_set);

/**
 * @brief Clears specific bits in the system event group.
 * @param bits_to_clear Bitmask of event bits to clear.
 */
void system_events_clear_bits(EventBits_t bits_to_clear);
