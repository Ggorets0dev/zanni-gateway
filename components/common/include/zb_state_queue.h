#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "zanni_protocol.h"

bool zb_state_queue_init(void);

bool zb_state_queue_send(zanni_message_t* msg);

bool zb_state_queue_receive(zanni_message_t* msg, uint32_t delay_ms);
