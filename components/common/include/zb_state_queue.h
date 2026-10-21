#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "zanni_protocol.h"

/**
 * @brief Initializes the Zigbee state message queue.
 * @return True if queue was created successfully, false otherwise.
 * @retval true Queue creation succeeded.
 * @retval false Queue creation failed.
 */
bool zb_state_queue_init(void);

/**
 * @brief Enqueues a message for Zigbee state processing.
 * @param msg Pointer to the Zanni protocol message to enqueue.
 * @return True if message was successfully enqueued, false otherwise.
 * @retval true Message successfully enqueued.
 * @retval false Queue is uninitialized or full.
 */
bool zb_state_queue_send(zanni_message_t* msg);

/**
 * @brief Dequeues a message from the Zigbee state message queue with timeout.
 * @param msg Pointer to message structure where received data will be stored.
 * @param delay_ms Maximum time to wait in milliseconds (or portMAX_DELAY).
 * @return True if message was successfully received, false otherwise.
 * @retval true Message successfully received.
 * @retval false Queue is uninitialized, invalid pointer, or timeout occurred.
 */
bool zb_state_queue_receive(zanni_message_t* msg, uint32_t delay_ms);
