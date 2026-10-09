#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/sockets.h"
#include "esp_log.h"

#include "zanni_protocol.h"
#include "netmap.h"
#include "module.h"
#include "system_events.h"
#include "wifi_task.h"
#include "wifi_init.h"

#define TAG                     g_module_tag[MODULE_WIFI]
#define WIFI_TASK_PERIOD_MS     5000

#define HOST_IP_ADDR            "192.168.1.118"
#define PORT                    9199

static int connect_to_node(char* host, int port)
{
    struct sockaddr_in dest_addr;
    inet_pton(AF_INET, host, &dest_addr.sin_addr);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock < 0)
    {
        ESP_LOGE(TAG, "Failed to create socket: errno %d", errno);
        return -1;
    }

    struct timeval timeout;
    timeout.tv_sec = 2; // 2 seconds timeout
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    ESP_LOGI(TAG, "Connecting to %s:%d", HOST_IP_ADDR, PORT);
    int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0)
    {
        ESP_LOGE(TAG, "Socket connection failed: errno %d", errno);
        close(sock);
        return -1;
    }

    return sock;
}

static bool send_tcp_message(int sock)
{
    static int counter;
    uint8_t payload[ZANNI_MSG_SIZE] = {0};
    zanni_message_t msg = {0};
    
    // Build JSON payload
    snprintf(msg.tag, sizeof(msg.tag),
             "DEV1");

    msg.status = counter;
    msg.device_type = ZANNI_VIRT_DEV_TOGGLE;

    ++counter;

    zanni_message_encode(&msg, payload, sizeof(payload));

    ESP_LOGI(TAG, "Socket connected, sending data...");
    int err = send(sock, payload, ZANNI_MSG_SIZE, 0);
    if (err < 0)
    {
        ESP_LOGE(TAG, "Failed to send data: errno %d", errno);
        return false;
    }
    
    ESP_LOGI(TAG, "Message sent: %s", payload);
    
    return true;
}

void wifi_task(void *pvParameter)
{
    zanni_wifi_init();

    int sock = -1;

    for (;;)
    {

        if (system_events_wait_bits(SYS_EVENT_WIFI_CONNECTED_BIT, 0) == pdTRUE)
        {
            if (sock == -1)
            {
                sock = connect_to_node(HOST_IP_ADDR, PORT);
            }
            if (sock != -1)
            {
                if (!send_tcp_message(sock))
                {
                    ESP_LOGW(TAG, "Closing socket due to error");
                    close(sock);
                    sock = -1;
                }
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(WIFI_TASK_PERIOD_MS));
    }
}