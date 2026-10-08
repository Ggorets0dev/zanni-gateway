#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wifi_task.h"
#include "wifi_init.h"

void wifi_task(void *pvParameter)
{
    zanni_wifi_init();

    while (1) {
        // работа задачи

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}