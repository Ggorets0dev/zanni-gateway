#include "status_queue.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "status_queue.h"

/**
 * @brief Maximum capacity of the module status message queue.
 */
#define STATUS_QUEUE_LENGTH 10

static QueueHandle_t status_queue = NULL;

/**
 * @brief Initializes the module status message queue.
 * @return True if queue was created successfully, false otherwise.
 * @retval true Queue creation succeeded.
 * @retval false Queue creation failed.
 */
bool status_queue_init(void)
{
    status_queue = xQueueCreate(
        STATUS_QUEUE_LENGTH,
        sizeof(module_status_e)
    );

    return status_queue != NULL;
}

/**
 * @brief Enqueues a status message with a specified timeout.
 * @param status Status message structure to enqueue.
 * @param delay_ms Maximum time to wait in milliseconds (or portMAX_DELAY).
 * @return True if message was successfully enqueued, false otherwise.
 * @retval true Message successfully enqueued.
 * @retval false Queue is uninitialized or full.
 */
bool status_queue_send(status_msg_t status, uint32_t delay_ms)
{
    if (status_queue == NULL)
    {
        return false;
    }

    return xQueueSend(
        status_queue,
        &status,
        (delay_ms == portMAX_DELAY) ? portMAX_DELAY : pdMS_TO_TICKS(delay_ms)
    ) == pdPASS;
}

/**
 * @brief Dequeues a status message from the status queue with timeout.
 * @param status Pointer to status message structure where received data will be stored.
 * @param delay_ms Maximum time to wait in milliseconds (or portMAX_DELAY).
 * @return True if message was successfully received, false otherwise.
 * @retval true Message successfully received.
 * @retval false Queue is uninitialized, invalid pointer, or timeout occurred.
 */
bool status_queue_receive(status_msg_t* status, uint32_t delay_ms)
{
    if (status_queue == NULL || status == NULL)
    {
        return false;
    }

    return xQueueReceive(
        status_queue,
        status,
        (delay_ms == portMAX_DELAY) ? portMAX_DELAY : pdMS_TO_TICKS(delay_ms)
    ) == pdPASS;
}