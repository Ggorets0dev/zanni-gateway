#pragma once

#include <stdint.h>

/**
 * @brief Retrieves the current state of a toggle virtual device.
 * @param dev_inx Index of the virtual device.
 * @param state_out Pointer to boolean variable where the state will be stored.
 * @return Status code.
 * @retval 0 State successfully retrieved.
 * @retval 1 Invalid arguments or device is not of toggle type.
 */
int dev_toggle_get_state(const uint8_t dev_inx, bool* state_out);

/**
 * @brief Updates the state of a toggle virtual device.
 * @param dev_inx Index of the virtual device.
 * @param state New boolean state to set.
 * @return Status code.
 * @retval 0 State successfully updated.
 * @retval 1 Invalid arguments or device is not of toggle type.
 */
int dev_toggle_set_state(const uint8_t dev_inx, bool state);