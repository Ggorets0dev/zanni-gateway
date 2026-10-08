#include "zigbee_init.h"
#include "common.h"
#include <esp_zigbee.h>
#include "esp_coexist.h"

void zanni_zigbee_init(void)
{
    esp_coex_wifi_i154_enable();

    esp_zigbee_config_t config = 
    {
        .device_config = 
        {
            .device_type = EZB_NWK_DEVICE_TYPE_ROUTER,
            .install_code_policy = false,
            .zczr_config = 
            {
                .max_children = 10
            }
        },
        .platform_config = 
        {
            .radio_config = 
            {
                .radio_mode = ESP_ZIGBEE_RADIO_MODE_NATIVE
            },
            .storage_partition_name = "nvs"
        }
    };

    esp_zigbee_init(&config);
}