#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "net_queue.h"

#define NET_QUEUE_LENGTH 10

static QueueHandle_t net_queue = NULL;

bool net_queue_init(void)
{
    net_queue = xQueueCreate(
        NET_QUEUE_LENGTH,
        sizeof(gate_msg_t)
    );

    return net_queue != NULL;
}

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