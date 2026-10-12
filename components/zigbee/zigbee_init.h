#pragma once

#include <stdint.h>
#include "ezbee/af.h"
#include "zanni_protocol.h"

void zanni_zigbee_init(void);

void zigbee_apply_spoofing(uint8_t endpoint, zanni_virt_dev_e dev_type);

int zigbee_init_endpoints(ezb_af_device_desc_t dev_desc);

int zigbee_add_endpoint(ezb_af_device_desc_t dev_desc, const zanni_virt_dev_e dev_type, uint8_t ep_id);