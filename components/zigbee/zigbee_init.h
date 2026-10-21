#pragma once

#include <stdint.h>
#include "ezbee/af.h"
#include "zanni_protocol.h"

/**
 * @brief Initializes coexistence and configures the Zigbee stack as a router device.
 */
void zanni_zigbee_init(void);

/**
 * @brief Configures IEEE 802.15.4 radio power management and transmit power.
 */
void zanni_zb_pm_setup(void);

/**
 * @brief Applies manufacturer name and model identifier spoofing attributes on the specified endpoint.
 * @param endpoint Zigbee endpoint identifier.
 */
void zigbee_apply_spoofing(uint8_t endpoint);

/**
 * @brief Initializes Zigbee endpoint clusters and registers them in the device descriptor.
 * @param dev_desc Application framework device descriptor.
 * @return Status code.
 * @retval 0 Endpoints successfully initialized.
 * @retval 1 Basic cluster creation failed.
 */
int zigbee_init_endpoints(ezb_af_device_desc_t dev_desc);