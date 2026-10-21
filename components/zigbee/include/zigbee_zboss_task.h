#pragma once

/**
 * @brief FreeRTOS task function running the Zigbee stack (ZBOSS) main event loop.
 * @param pvParameters Pointer to task parameters (unused).
 */
void zigbee_zboss_task(void *pvParameters);