#pragma once

#include <stdint.h>
#include <stdbool.h>

#define BRIGHTNESS_BASE_PCT     20
#define BRIGHTNESS_STEP_PCT     5

void zigbee_update_brightness(uint8_t endpoint, uint8_t level, bool is_set_attr);

void zigbee_update_on_off(uint8_t endpoint, bool state, bool is_set_attr);

uint8_t zigbee_level_to_pct(uint8_t level);

uint8_t zigbee_state_to_level(uint8_t dev_inx, bool state);

uint8_t zigbee_pct_to_level(uint8_t pct);