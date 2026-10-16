

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "dev_state.h"
#include "netmap.h"
#include "attributes.h"

static SemaphoreHandle_t dev_state_sem;

static void* dev_states[DEVS_COUNT] = {0};

static size_t dev_state_size[ZANNI_VIRT_DEV_MAX] =
{
    [ZANNI_VIRT_DEV_TOGGLE] = sizeof(bool)
};

__CONSTRUCTOR void dev_states_init(void)
{
    for (uint8_t i = 0; i < DEVS_COUNT; ++i)
    {
        dev_states[i] = malloc(dev_state_size[zanni_virt_devs[i].type]);
    }
}

static void operate_state(void* to, const void* from, const uint8_t dev_inx)
{
    const size_t state_size = dev_state_size[zanni_virt_devs[dev_inx].type];

    xSemaphoreTake(dev_state_sem, portMAX_DELAY);

    memcpy(to, from, state_size);

    xSemaphoreGive(dev_state_sem);
}

static int dev_state_set(const uint8_t dev_inx, const void* state)
{
    if (!state || dev_inx >= DEVS_COUNT) return 1;

    operate_state(&dev_states[dev_inx], state, dev_inx);

    // Add more SET hooks if needed

    return 0;
}

static int dev_state_get(const uint8_t dev_inx, void* state_out)
{
    if (!state_out || dev_inx >= DEVS_COUNT) return 1;

    operate_state(state_out, &dev_states[dev_inx], dev_inx);

    // Add more GET hooks if needed

    return 0;
}

int dev_toggle_get_state(const uint8_t dev_inx, bool* state_out)
{
    if (zanni_virt_devs[dev_inx].type != ZANNI_VIRT_DEV_TOGGLE) return 1;
    return dev_state_get(dev_inx, state_out);
}

int dev_toggle_set_state(const uint8_t dev_inx, bool state)
{
    if (zanni_virt_devs[dev_inx].type != ZANNI_VIRT_DEV_TOGGLE) return 1;
    return dev_state_set(dev_inx, &state);
}