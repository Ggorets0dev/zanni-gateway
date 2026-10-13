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
#include "net_queue.h"
#include "status_queue.h"

#define TAG                         g_module_tag[MODULE_WIFI]
#define WIFI_TASK_READ_DELAY        1000u
#define WIFI_TASK_WAIT_DELAY        1000u

#define KEEP_ALIVE_IDLE_SEC         30
#define KEEP_ALIVE_INTERVAL_SEC     10
#define KEEP_ALIVE_ATTEMPTS_CNT     3

static void configure_keepalive(int sock)
{
    int keep_alive_enable = 1; 
    int keep_idle         = KEEP_ALIVE_IDLE_SEC;        // sec
    int keep_interval     = KEEP_ALIVE_INTERVAL_SEC;    // sec
    int keep_count        = KEEP_ALIVE_ATTEMPTS_CNT;    // times

    setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keep_alive_enable, sizeof(keep_alive_enable));
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keep_idle, sizeof(keep_idle));
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keep_interval, sizeof(keep_interval));
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keep_count, sizeof(keep_count));
}

static int connect_to_node(const char* host, int port)
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

    configure_keepalive(sock);

    struct timeval timeout;
    timeout.tv_sec = 1;     // timeout
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    ESP_LOGI(TAG, "Connecting to %s:%d", host, port);
    int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    
    if (err != 0)
    {
        ESP_LOGE(TAG, "Socket connection failed: errno %d", errno);
        close(sock);
        return -1;
    }

    return sock;
}

static bool send_msg_to_node(const int sock, zanni_message_t* msg)
{
    uint8_t payload[ZANNI_MSG_SIZE] = {0};

    zanni_message_encode(msg, payload, sizeof(payload));

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

static void close_connections(int socks[NODES_COUNT])
{
    for (uint16_t i = 0; i < NODES_COUNT; ++i) 
    {
        if (socks[i] == -1) continue;

        close(socks[i]);
        socks[i] = -1;
    }
}

void wifi_task(void *pvParameter)
{
    status_msg_t status_msg = 
    {
        .module = MODULE_WIFI,
        .status = MODULE_STATUS_RX
    };

    bool is_started = false;
    bool rc = false;
    bool prev_conn_state = false;
    int socks[NODES_COUNT] = {0};
    gate_msg_t msg = {0};

    // Init socks with 'no connection' value
    for (uint16_t i = 0; i < NODES_COUNT; ++i) socks[i] = -1;
    
    zanni_wifi_init();

    for (;;)
    {
        bool is_all_connected = true;

        if (system_events_wait_bits(SYS_EVENT_WIFI_CONNECTED_BIT, WIFI_TASK_WAIT_DELAY) != pdTRUE)
        {
            if (is_started)
            {
                close_connections(socks);
            }

            continue;
        }
        
        is_started = true;

        // COLLECT CONNECT STATUS
        for (uint16_t i = 0; i < NODES_COUNT; ++i)
        {
            if (socks[i] == -1)
            {
                // Connection is not ready
                socks[i] = connect_to_node(zanni_nodes[i].ip, zanni_nodes[i].port);
            }

            is_all_connected &= (socks[i] != -1);
        }

        // SUMBIT CONNECT STATUS
        if (prev_conn_state != is_all_connected)
        {
            if (is_all_connected)   system_events_set_bits(SYS_EVENT_NODES_CONNECTED_BIT);
            else                    system_events_clear_bits(SYS_EVENT_NODES_CONNECTED_BIT);
        }
        prev_conn_state = is_all_connected;

        // TRANSMIT MESSAGES
        const bool is_any = net_queue_receive(&msg, WIFI_TASK_READ_DELAY);

        if (is_any && msg.node_id - 1 < DEVS_COUNT)
        {
            const int curr_sock = socks[msg.node_id - 1];

            if (curr_sock == -1) continue;

            rc = send_msg_to_node(curr_sock, &msg.payload);
            rc = status_queue_send(status_msg, 0);
        }
    }
}