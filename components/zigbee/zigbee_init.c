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

#define TAG     g_module_config[MODULE_ZIGBEE_ZBOSS].tag

static char s_kojima_manuf[] = "\x10""_TZ3210_b3kiq1i0";
static char s_kojima_model[] = "\x07""TS0505B";

static void create_kojima_light_endpoint(ezb_zcl_cluster_desc_t* basic, ezb_af_ep_desc_t* desc, uint8_t ep_id)
{
    ezb_zha_color_dimmable_light_config_t light_cfg = EZB_ZHA_COLOR_DIMMABLE_LIGHT_CONFIG();
    light_cfg.on_off_cfg.on_off = false;
    light_cfg.level_cfg.current_level = 254;

    *desc = ezb_zha_create_color_dimmable_light(ep_id, &light_cfg);
    ezb_af_ep_desc_set_app_device_id(*desc, 0x010D); // Extended Color Light (0x010D)
    *basic = ezb_af_endpoint_get_cluster_desc(*desc, EZB_ZCL_CLUSTER_ID_BASIC, EZB_ZCL_CLUSTER_SERVER);

    ezb_zcl_cluster_desc_t level_cluster = ezb_af_endpoint_get_cluster_desc(*desc, EZB_ZCL_CLUSTER_ID_LEVEL, EZB_ZCL_CLUSTER_SERVER);
    if (level_cluster) 
    {
        static const uint8_t min_lvl = 1;
        static const uint8_t max_lvl = 254;
        ezb_zcl_level_cluster_desc_add_attr(level_cluster, EZB_ZCL_ATTR_LEVEL_MIN_LEVEL_ID, &min_lvl);
        ezb_zcl_level_cluster_desc_add_attr(level_cluster, EZB_ZCL_ATTR_LEVEL_MAX_LEVEL_ID, &max_lvl);
    }
}

void zigbee_apply_spoofing(uint8_t endpoint)
{
    esp_err_t r1 = ezb_zcl_set_attr_value(
        endpoint,
        EZB_ZCL_CLUSTER_ID_BASIC,
        EZB_ZCL_CLUSTER_SERVER,
        EZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID,
        EZB_ZCL_STD_MANUF_CODE,
        s_kojima_manuf,
        false
    );
    esp_err_t r2 = ezb_zcl_set_attr_value(
        endpoint,
        EZB_ZCL_CLUSTER_ID_BASIC,
        EZB_ZCL_CLUSTER_SERVER,
        EZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID,
        EZB_ZCL_STD_MANUF_CODE,
        s_kojima_model,
        false
    );

    ESP_LOGI(TAG, "Spoofing: ManufName=%s ModelID=%s (r1=%d r2=%d)",
        s_kojima_manuf + 1, s_kojima_model + 1, r1, r2);
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
    ezb_zcl_cluster_desc_t basic = NULL;
    ezb_af_ep_desc_t desc = NULL;

    create_kojima_light_endpoint(&basic, &desc, 1);

    if (basic) 
    {
        ezb_zcl_basic_cluster_desc_add_attr(basic, EZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID, s_kojima_manuf);
        ezb_zcl_basic_cluster_desc_add_attr(basic, EZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID, s_kojima_model);
    }
    else
    {
        return 1;
    }

    ezb_af_device_add_endpoint_desc(dev_desc, desc);

    return 0;
}