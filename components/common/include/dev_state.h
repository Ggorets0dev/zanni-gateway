#pragma once

#include <stdint.h>

int dev_toggle_get_state(const uint8_t dev_inx, bool* state_out);

int dev_toggle_set_state(const uint8_t dev_inx, bool state);