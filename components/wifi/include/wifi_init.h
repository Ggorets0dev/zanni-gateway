#pragma once

#include "freertos/FreeRTOS.h"

/**
 * @brief Initializes network interface, Wi-Fi driver, event handlers, and connects in station mode.
 */
void zanni_wifi_init(void);