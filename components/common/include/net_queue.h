#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "zanni_protocol.h"

typedef struct 
{
    zanni_message_t payload;
    uint32_t node_id;
} gate_msg_t;

bool net_queue_init(void);

bool net_queue_send(gate_msg_t* msg);

bool net_queue_receive(gate_msg_t* msg, uint32_t delay_ms);
