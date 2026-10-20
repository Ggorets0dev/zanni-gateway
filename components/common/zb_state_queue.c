#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "zb_state_queue.h"

#define ZB_STATE_QUEUE_LENGTH   5

static QueueHandle_t zb_state_queue = NULL;

bool zb_state_queue_init(void)
{
    zb_state_queue = xQueueCreate(
        ZB_STATE_QUEUE_LENGTH,
        sizeof(zanni_message_t)
    );

    return zb_state_queue != NULL;
}

bool zb_state_queue_send(zanni_message_t* msg)
{
    if (zb_state_queue == NULL)
    {
        return false;
    }

    return xQueueSend(
        zb_state_queue,
        msg,
        portMAX_DELAY
    ) == pdPASS;
}

bool zb_state_queue_receive(zanni_message_t* msg, uint32_t delay_ms)
{
    if (zb_state_queue == NULL || msg == NULL)
    {
        return false;
    }

    return xQueueReceive(
        zb_state_queue,
        msg,
        (delay_ms == portMAX_DELAY) ? portMAX_DELAY : pdMS_TO_TICKS(delay_ms)
    ) == pdPASS;
}