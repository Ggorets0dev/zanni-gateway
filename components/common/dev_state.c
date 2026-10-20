

#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "dev_state.h"
#include "netmap.h"
#include "attributes.h"

typedef void (*dev_state_init_t)(void*);

static void dev_state_toggle_init(void* ptr);

// ================
// VARIABLES
// ================
static SemaphoreHandle_t dev_state_sem;

static void* dev_states[DEVS_COUNT] = {0};

static size_t dev_state_size[ZANNI_VIRT_DEV_MAX] =
{
    [ZANNI_VIRT_DEV_TOGGLE] = sizeof(bool)
};

static dev_state_init_t dev_state_init[ZANNI_VIRT_DEV_MAX] =
{
    [ZANNI_VIRT_DEV_TOGGLE] = dev_state_toggle_init
};
// ================

__CONSTRUCTOR void dev_states_init(void)
{
    dev_state_sem = xSemaphoreCreateMutex();
    configASSERT(dev_state_sem != NULL);

    for (uint8_t i = 0; i < DEVS_COUNT; ++i)
    {
        dev_states[i] = malloc(dev_state_size[zanni_virt_devs[i].type]);
        dev_state_init[zanni_virt_devs[i].type](dev_states[i]);
    }
}

static void dev_state_toggle_init(void* ptr)
{
    bool* value = ptr;
    *value = false;
}

static void operate_state(void* to, const void* from, const uint8_t dev_inx)
{
    const size_t state_size = dev_state_size[zanni_virt_devs[dev_inx].type];

    ESP_LOGI("DEV_STATE", "Operating with dev state: [dev_inx=%u]", dev_inx);

    xSemaphoreTake(dev_state_sem, portMAX_DELAY);

    memcpy(to, from, state_size);

    xSemaphoreGive(dev_state_sem);
}

static int dev_state_set(const uint8_t dev_inx, const void* state)
{
    operate_state(dev_states[dev_inx], state, dev_inx);

    // Add more SET hooks if needed

    return 0;
}

static int dev_state_get(const uint8_t dev_inx, void* state_out)
{
    operate_state(state_out, dev_states[dev_inx], dev_inx);

    // Add more GET hooks if needed

    return 0;
}

int dev_toggle_get_state(const uint8_t dev_inx, bool* state_out)
{
    if (!state_out || dev_inx >= DEVS_COUNT)                    return 1;
    if (zanni_virt_devs[dev_inx].type != ZANNI_VIRT_DEV_TOGGLE) return 1;

    return dev_state_get(dev_inx, state_out);
}

int dev_toggle_set_state(const uint8_t dev_inx, bool state)
{
    if (dev_inx >= DEVS_COUNT)                                  return 1;
    if (zanni_virt_devs[dev_inx].type != ZANNI_VIRT_DEV_TOGGLE) return 1;
    
    return dev_state_set(dev_inx, &state);
}