#pragma once

#include <stdint.h>
#include "ezbee/af.h"
#include "zanni_protocol.h"

void zanni_zigbee_init(void);

void zigbee_apply_spoofing(uint8_t endpoint);

int zigbee_init_endpoints(ezb_af_device_desc_t dev_desc);