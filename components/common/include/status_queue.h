#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "module.h"

/**
 * @brief Enumeration of operational and communication status codes for modules.
 */
typedef enum 
{
    MODULE_STATUS_OK,
    MODULE_STATUS_CONF,
    MODULE_STATUS_RX,
    MODULE_STATUS_TX,
    MODULE_STATUS_ERROR,
    MODULE_STATUS_MAX,
} module_status_e;

/**
 * @brief Status update message envelope containing module identifier and status.
 */
typedef struct 
{
    uint16_t module;
    uint16_t status;
} status_msg_t;

/**
 * @brief Initializes the module status message queue.
 * @return True if queue was created successfully, false otherwise.
 * @retval true Queue creation succeeded.
 * @retval false Queue creation failed.
 */
bool status_queue_init(void);

/**
 * @brief Enqueues a status message with a specified timeout.
 * @param status Status message structure to enqueue.
 * @param delay_ms Maximum time to wait in milliseconds (or portMAX_DELAY).
 * @return True if message was successfully enqueued, false otherwise.
 * @retval true Message successfully enqueued.
 * @retval false Queue is uninitialized or full.
 */
bool status_queue_send(status_msg_t status, uint32_t delay_ms);

/**
 * @brief Dequeues a status message from the status queue with timeout.
 * @param status Pointer to status message structure where received data will be stored.
 * @param delay_ms Maximum time to wait in milliseconds (or portMAX_DELAY).
 * @return True if message was successfully received, false otherwise.
 * @retval true Message successfully received.
 * @retval false Queue is uninitialized, invalid pointer, or timeout occurred.
 */
bool status_queue_receive(status_msg_t* status, uint32_t delay_ms);
