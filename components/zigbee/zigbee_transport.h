#pragma once

#include <stdint.h>
#include <stdbool.h>

void zigbee_update_brightness(uint8_t endpoint, uint8_t level, bool is_set_attr);
void zigbee_update_on_off(uint8_t endpoint, bool state, bool is_set_attr);