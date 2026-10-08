#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "module.h"

bool status_queue_init(void);

bool status_queue_send(module_status_e status);

bool status_queue_receive(module_status_e* status, uint32_t delay);
