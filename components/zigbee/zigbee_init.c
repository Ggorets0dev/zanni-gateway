#include <esp_zigbee.h>
#include "ezbee/zha.h"
#include "ezbee/zcl.h"
#include "ezbee/bdb.h"
#include "ezbee/zdo.h"
#include "ezbee/core.h"
#include "esp_coexist.h"
#include "zigbee_init.h"
#include "netmap.h"
#include "module.h"

#define TAG     g_module_tag[MODULE_ZIGBEE]


static char* spoofing_records[ZANNI_VIRT_DEV_MAX][2] = 
{
    [ZANNI_VIRT_DEV_TOGGLE] = {"\x06""SONOFF", "\x09""BASICZBR3"}
};

static void create_toggle_endpoint(ezb_zcl_cluster_desc_t* basic, ezb_af_ep_desc_t* desc, uint8_t ep_id)
{
    ezb_zha_on_off_light_config_t light_cfg = EZB_ZHA_ON_OFF_LIGHT_CONFIG();
    *desc = ezb_zha_create_on_off_light(ep_id, &light_cfg);
    *basic = ezb_af_endpoint_get_cluster_desc(*desc, EZB_ZCL_CLUSTER_ID_BASIC, EZB_ZCL_CLUSTER_SERVER);
}

void zigbee_apply_spoofing(uint8_t endpoint, zanni_virt_dev_e dev_type)
{
    if (dev_type >= ZANNI_VIRT_DEV_MAX || spoofing_records[dev_type][0] == NULL || spoofing_records[dev_type][1] == NULL)
    {
        ESP_LOGW(TAG, "No spoofing record for dev_type %d", dev_type);
        return;
    }

    esp_err_t r1 = ezb_zcl_set_attr_value(
        endpoint,
        EZB_ZCL_CLUSTER_ID_BASIC,
        EZB_ZCL_CLUSTER_SERVER,
        EZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID,
        EZB_ZCL_STD_MANUF_CODE,
        spoofing_records[dev_type][0],
        false
    );
    esp_err_t r2 = ezb_zcl_set_attr_value(
        endpoint,
        EZB_ZCL_CLUSTER_ID_BASIC,
        EZB_ZCL_CLUSTER_SERVER,
        EZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID,
        EZB_ZCL_STD_MANUF_CODE,
        spoofing_records[dev_type][1],
        false
    );

    ESP_LOGI(TAG, "Spoofing: ManufName=%s ModelID=%s (r1=%d r2=%d)",
        spoofing_records[dev_type][0] + 1, spoofing_records[dev_type][1] + 1, r1, r2);
}

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

int zigbee_init_endpoints(ezb_af_device_desc_t dev_desc)
{
    int rc = 0;

    for (uint8_t i = 0; i < DEVS_COUNT; ++i)
    {
        rc = zigbee_add_endpoint(dev_desc, zanni_virt_devs[i].type, i + 1);

        if (rc != 0)
        {
            return 1;
        }
    }

    return 0;
}

int zigbee_add_endpoint(ezb_af_device_desc_t dev_desc, const zanni_virt_dev_e dev_type, uint8_t ep_id)
{
    ezb_zcl_cluster_desc_t basic = NULL;
    ezb_af_ep_desc_t desc = NULL;

    if (dev_type == ZANNI_VIRT_DEV_TOGGLE)
    {
        create_toggle_endpoint(&basic, &desc, ep_id);
    }
    else
    {
        return 1;
    }

    if (basic) 
    {
        ezb_zcl_basic_cluster_desc_add_attr(basic, EZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID, spoofing_records[dev_type][0]);
        ezb_zcl_basic_cluster_desc_add_attr(basic, EZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID, spoofing_records[dev_type][1]);
    }
    else
    {
        return 1;
    }

    ezb_af_device_add_endpoint_desc(dev_desc, desc);

    return 0;
}