#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "zanni_protocol.h"

/**
 * @brief Gateway network message envelope containing payload and target node ID.
 */
typedef struct 
{
    zanni_message_t payload;
    uint32_t node_id;
} gate_msg_t;

/**
 * @brief Initializes the network message queue.
 * @return True if queue was created successfully, false otherwise.
 * @retval true Queue creation succeeded.
 * @retval false Queue creation failed.
 */
bool net_queue_init(void);

/**
 * @brief Enqueues a message for network transmission.
 * @param msg Pointer to the gateway message structure to enqueue.
 * @return True if message was successfully sent to the queue, false otherwise.
 * @retval true Message successfully enqueued.
 * @retval false Queue is uninitialized or full.
 */
bool net_queue_send(gate_msg_t* msg);

/**
 * @brief Dequeues a message from the network message queue with timeout.
 * @param msg Pointer to the gateway message structure where received data will be stored.
 * @param delay_ms Maximum time to wait in milliseconds (or portMAX_DELAY).
 * @return True if message was successfully received, false otherwise.
 * @retval true Message successfully received.
 * @retval false Queue is uninitialized, invalid arguments, or timeout occurred.
 */
bool net_queue_receive(gate_msg_t* msg, uint32_t delay_ms);
