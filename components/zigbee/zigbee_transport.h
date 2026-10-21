#pragma once

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Base brightness percentage threshold used for encoding virtual device states.
 */
#define BRIGHTNESS_BASE_PCT     20

/**
 * @brief Brightness percentage step size per virtual device state.
 */
#define BRIGHTNESS_STEP_PCT     5

/**
 * @brief Updates the current level attribute on the Level Control cluster and reports it.
 * @param endpoint Zigbee endpoint identifier.
 * @param level Brightness level value (0-254).
 * @param is_set_attr Flag indicating whether to set the local cluster attribute before reporting.
 */
void zigbee_update_brightness(uint8_t endpoint, uint8_t level, bool is_set_attr);

/**
 * @brief Updates the On/Off attribute on the On/Off cluster and reports it.
 * @param endpoint Zigbee endpoint identifier.
 * @param state Target on/off boolean state.
 * @param is_set_attr Flag indicating whether to set the local cluster attribute before reporting.
 */
void zigbee_update_on_off(uint8_t endpoint, bool state, bool is_set_attr);

/**
 * @brief Converts raw Zigbee level (25..254) to brightness percentage (1..100%).
 * @param level Raw Zigbee level value.
 * @return Equivalent percentage value.
 */
uint8_t zigbee_level_to_pct(uint8_t level);

/**
 * @brief Converts a virtual device index and boolean state to a corresponding Zigbee level.
 * @param dev_inx Index of the virtual device.
 * @param state Boolean state of the device.
 * @return Corresponding Zigbee level value.
 */
uint8_t zigbee_state_to_level(uint8_t dev_inx, bool state);

/**
 * @brief Converts brightness percentage (1..100%) to raw Zigbee level (25..254).
 * @param pct Brightness percentage value.
 * @return Corresponding Zigbee level value.
 */
uint8_t zigbee_pct_to_level(uint8_t pct);