#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "module.h"

typedef enum 
{
    MODULE_STATUS_OK,
    MODULE_STATUS_CONF,
    MODULE_STATUS_RX,
    MODULE_STATUS_TX,
    MODULE_STATUS_ERROR,
    MODULE_STATUS_MAX,
} module_status_e;

typedef struct 
{
    uint16_t module;
    uint16_t status;
} status_msg_t;

bool status_queue_init(void);

bool status_queue_send(status_msg_t status, uint32_t delay_ms);

bool status_queue_receive(status_msg_t* status, uint32_t delay_ms);
