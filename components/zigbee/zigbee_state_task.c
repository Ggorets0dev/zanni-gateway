#include "esp_log.h"
#include "esp_zigbee.h"
#include "module.h"
#include "dev_state.h"
#include "netmap.h"
#include "zigbee_state_task.h"
#include "zigbee_transport.h"
#include "zanni_protocol.h"
#include "zb_state_queue.h"

#define TAG     g_module_config[MODULE_ZIGBEE_STATE].tag

void zigbee_state_task(void *pvParameters) 
{
    zanni_message_t msg = {0};
    bool status = false;
    bool dev_state = false;

    for (;;) 
    {
        status = zb_state_queue_receive(&msg, portMAX_DELAY);

        if (!status) continue;

        ESP_LOGI(TAG, "Zigbee state recieved: [dev_tag=%s, state=%zu]", msg.tag, msg.payload.dev_status);

        esp_zigbee_lock_acquire(portMAX_DELAY);
        
        const int dev_inx = zanni_virt_dev_find_index(msg.tag);

        if (dev_inx == -1)
        {
            ESP_LOGE(TAG, "Failed to find device with provided tag for zigbee state update: [dev_tag=%s]", msg.tag);
            continue;
        }

        dev_toggle_get_state(dev_inx, &dev_state);

        dev_state = msg.payload.exec_status ? !dev_state : dev_state;

        const uint8_t level = zigbee_state_to_level((uint8_t)dev_inx, dev_state);
        zigbee_update_brightness(1, level, false);
        
        dev_toggle_set_state(dev_inx, dev_state);

        esp_zigbee_lock_release();
    }
}

