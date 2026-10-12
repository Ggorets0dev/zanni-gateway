#include "zigbee_transport.h"

#include "esp_zigbee.h"
#include "ezbee/zha.h"
#include "ezbee/zcl.h"
#include "ezbee/bdb.h"
#include "ezbee/zdo.h"
#include "ezbee/af.h"
#include "ezbee/core.h"
#include "ezbee/app_signals.h"

static void update_attr_and_report(uint8_t endpoint,
                                    uint16_t cluster_id,
                                    uint16_t attr_id,
                                    void *value_ptr,
                                    bool is_set_attr)
{
    if (is_set_attr)
    {
        ezb_zcl_set_attr_value(
            endpoint,
            cluster_id,
            EZB_ZCL_CLUSTER_SERVER,
            attr_id,
            EZB_ZCL_STD_MANUF_CODE,
            value_ptr,
            false
        );
    }


    ezb_zcl_report_attr_cmd_t report_cmd = 
    {
        .cmd_ctrl = 
        {
            .dst_addr = EZB_ADDRESS_SHORT(0x0000), 
            .dst_ep = 1,                           
            .src_ep = endpoint,                    
            .cluster_id = cluster_id,
            .manuf_code = EZB_ZCL_STD_MANUF_CODE,
            .fc = 
            {
                .manuf_specific = 0,
                .direction = EZB_ZCL_CMD_DIRECTION_TO_CLI,
                .dis_default_rsp = 0
            }
        },
        .payload = 
        {
            .attr_id = attr_id
        }
    };

    ezb_zcl_report_attr_cmd_req(&report_cmd);
}

void zigbee_update_relay_state(uint8_t endpoint, bool state, bool is_set_attr) 
{
    update_attr_and_report(endpoint,
                            EZB_ZCL_CLUSTER_ID_ON_OFF,
                            EZB_ZCL_ATTR_ON_OFF_ON_OFF_ID,
                            &state,
                            is_set_attr);
}