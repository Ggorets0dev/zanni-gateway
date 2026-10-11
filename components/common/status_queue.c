#include "status_queue.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "status_queue.h"

#define STATUS_QUEUE_LENGTH 10

static QueueHandle_t status_queue = NULL;

bool status_queue_init(void)
{
    status_queue = xQueueCreate(
        STATUS_QUEUE_LENGTH,
        sizeof(module_status_e)
    );

    return status_queue != NULL;
}

bool status_queue_send(status_msg_t status)
{
    if (status_queue == NULL)
    {
        return false;
    }

    return xQueueSend(
        status_queue,
        &status,
        portMAX_DELAY
    ) == pdPASS;
}

bool status_queue_receive(status_msg_t* status, uint32_t delay_ms)
{
    if (status_queue == NULL || status == NULL)
    {
        return false;
    }

    return xQueueReceive(
        status_queue,
        status,
        pdMS_TO_TICKS(delay_ms)
    ) == pdPASS;
}