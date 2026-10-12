#pragma once

#include <stdint.h>
#include <stdbool.h>

void zigbee_update_relay_state(uint8_t endpoint, bool state, bool is_set_attr);