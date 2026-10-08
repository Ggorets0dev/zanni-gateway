#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/sockets.h"
#include "esp_log.h"

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

static bool send_tcp_message(int sock, int endpoint, bool state, int level)
{
    char payload[256];
    // Build JSON payload
    snprintf(payload, sizeof(payload),
             "{\"source\": \"esp32_c6_bridge\", \"endpoint\": %d, \"action\": \"set_state\", "
             "\"payload\": {\"state\": %d, \"level\": %d}}\n",
             endpoint, state ? 1 : 0, level);

    ESP_LOGI(TAG, "Socket connected, sending data...");
    int err = send(sock, payload, strlen(payload), 0);
    if (err < 0)
    {
        ESP_LOGE(TAG, "Failed to send data: errno %d", errno);
        return false;
    }
    
    ESP_LOGI(TAG, "Message sent: %s", payload);

    // Optionally: wait for response from server ({"status": "ok", ...})
    char rx_buffer[128];
    int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
    if (len > 0)
    {
        rx_buffer[len] = 0; // Null-terminate
        ESP_LOGI(TAG, "Server response: %s", rx_buffer);
        return true;
    }
    else
    {
        ESP_LOGW(TAG, "Failed to receive response or timeout: len %d, errno %d", len, errno);
        return false;
    }
}

void wifi_task(void *pvParameter)
{
    zanni_wifi_init();

    int sock = -1;
    int level = 0;
    
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
                if (!send_tcp_message(sock, 0, 0, level))
                {
                    ESP_LOGW(TAG, "Closing socket due to error");
                    close(sock);
                    sock = -1;
                }
            }
        }
        
        ++level;
        vTaskDelay(pdMS_TO_TICKS(WIFI_TASK_PERIOD_MS));
    }
}