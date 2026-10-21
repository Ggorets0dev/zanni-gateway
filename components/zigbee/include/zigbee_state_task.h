#pragma once

/**
 * @brief FreeRTOS task function for handling Zigbee state synchronization and reporting.
 * @param pvParameters Pointer to task parameters (unused).
 */
void zigbee_state_task(void *pvParameters);