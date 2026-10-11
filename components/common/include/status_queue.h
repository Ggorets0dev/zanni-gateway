#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "module.h"

typedef struct 
{
    uint16_t module;
    uint16_t status;
} status_msg_t;

bool status_queue_init(void);

bool status_queue_send(status_msg_t status);

bool status_queue_receive(status_msg_t* status, uint32_t delay_ms);
