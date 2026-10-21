#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "net_queue.h"

/**
 * @brief Maximum capacity of the network message queue.
 */
#define NET_QUEUE_LENGTH 10

static QueueHandle_t net_queue = NULL;

/**
 * @brief Initializes the network message queue.
 * @return True if queue was created successfully, false otherwise.
 * @retval true Queue creation succeeded.
 * @retval false Queue creation failed.
 */
bool net_queue_init(void)
{
    net_queue = xQueueCreate(
        NET_QUEUE_LENGTH,
        sizeof(gate_msg_t)
    );

    return net_queue != NULL;
}

/**
 * @brief Enqueues a message for network transmission.
 * @param msg Pointer to the gateway message structure to enqueue.
 * @return True if message was successfully sent to the queue, false otherwise.
 * @retval true Message successfully enqueued.
 * @retval false Queue is uninitialized or full.
 */
bool net_queue_send(gate_msg_t* msg)
{
    if (net_queue == NULL)
    {
        return false;
    }

    return xQueueSend(
        net_queue,
        msg,
        portMAX_DELAY
    ) == pdPASS;
}

/**
 * @brief Dequeues a message from the network message queue with timeout.
 * @param msg Pointer to the gateway message structure where received data will be stored.
 * @param delay_ms Maximum time to wait in milliseconds (or portMAX_DELAY).
 * @return True if message was successfully received, false otherwise.
 * @retval true Message successfully received.
 * @retval false Queue is uninitialized, invalid arguments, or timeout occurred.
 */
bool net_queue_receive(gate_msg_t* msg, uint32_t delay_ms)
{
    if (net_queue == NULL || msg == NULL)
    {
        return false;
    }

    return xQueueReceive(
        net_queue,
        msg,
        (delay_ms == portMAX_DELAY) ? portMAX_DELAY : pdMS_TO_TICKS(delay_ms)
    ) == pdPASS;
}