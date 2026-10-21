

#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "dev_state.h"
#include "netmap.h"
#include "attributes.h"

/**
 * @brief Function pointer type for device state initialization.
 * @param ptr Pointer to the device state memory block.
 */
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

/**
 * @brief Constructor function initializing the device state mutex and allocating device states.
 */
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

/**
 * @brief Initializes toggle device state to false.
 * @param ptr Pointer to boolean state memory.
 */
static void dev_state_toggle_init(void* ptr)
{
    bool* value = ptr;
    *value = false;
}

/**
 * @brief Thread-safe memory copy of device state using mutex synchronization.
 * @param to Destination pointer.
 * @param from Source pointer.
 * @param dev_inx Index of the virtual device.
 */
static void operate_state(void* to, const void* from, const uint8_t dev_inx)
{
    const size_t state_size = dev_state_size[zanni_virt_devs[dev_inx].type];

    ESP_LOGI("DEV_STATE", "Operating with dev state: [dev_inx=%u]", dev_inx);

    xSemaphoreTake(dev_state_sem, portMAX_DELAY);

    memcpy(to, from, state_size);

    xSemaphoreGive(dev_state_sem);
}

/**
 * @brief Sets the state of a virtual device.
 * @param dev_inx Index of the virtual device.
 * @param state Pointer to new state data.
 * @return Status code.
 * @retval 0 Operation succeeded.
 */
static int dev_state_set(const uint8_t dev_inx, const void* state)
{
    operate_state(dev_states[dev_inx], state, dev_inx);

    // Add more SET hooks if needed

    return 0;
}

/**
 * @brief Retrieves the state of a virtual device.
 * @param dev_inx Index of the virtual device.
 * @param state_out Pointer to destination buffer for the state.
 * @return Status code.
 * @retval 0 Operation succeeded.
 */
static int dev_state_get(const uint8_t dev_inx, void* state_out)
{
    operate_state(state_out, dev_states[dev_inx], dev_inx);

    // Add more GET hooks if needed

    return 0;
}

/**
 * @brief Retrieves the current state of a toggle virtual device.
 * @param dev_inx Index of the virtual device.
 * @param state_out Pointer to boolean variable where the state will be stored.
 * @return Status code.
 * @retval 0 State successfully retrieved.
 * @retval 1 Invalid arguments or device is not of toggle type.
 */
int dev_toggle_get_state(const uint8_t dev_inx, bool* state_out)
{
    if (!state_out || dev_inx >= DEVS_COUNT)                    return 1;
    if (zanni_virt_devs[dev_inx].type != ZANNI_VIRT_DEV_TOGGLE) return 1;

    return dev_state_get(dev_inx, state_out);
}

/**
 * @brief Updates the state of a toggle virtual device.
 * @param dev_inx Index of the virtual device.
 * @param state New boolean state to set.
 * @return Status code.
 * @retval 0 State successfully updated.
 * @retval 1 Invalid arguments or device is not of toggle type.
 */
int dev_toggle_set_state(const uint8_t dev_inx, bool state)
{
    if (dev_inx >= DEVS_COUNT)                                  return 1;
    if (zanni_virt_devs[dev_inx].type != ZANNI_VIRT_DEV_TOGGLE) return 1;
    
    return dev_state_set(dev_inx, &state);
}