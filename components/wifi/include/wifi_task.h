#pragma once

/**
 * @brief FreeRTOS task handling TCP connections to network nodes and forwarding messages between queues and nodes.
 * @param pvParameter Pointer to task parameters (unused).
 */
void wifi_task(void *pvParameter);