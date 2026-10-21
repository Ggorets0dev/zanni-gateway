#pragma once

#define MODULE_START(id, task_fc, arg) \
    xTaskCreate(&task_fc, g_module_config[id].tag, g_module_config[id].stack_size, (arg), g_module_config[id].priority, NULL);

typedef enum
{
    MODULE_STATUS,
    MODULE_WIFI,
    MODULE_ZIGBEE_ZBOSS,
    MODULE_ZIGBEE_STATE,
#ifdef IS_MONITOR_TASK_ENABLED
    MODULE_MONITOR,
#endif
    MODULE_MAX
} module_e;

typedef struct
{
    const char *tag;
    uint32_t stack_size;
    uint32_t priority;
} module_config_t;

static const module_config_t g_module_config[MODULE_MAX] =
{
    [MODULE_STATUS] = {
        .tag = "STATUS",
        .stack_size = 2048,
        .priority = 4,
    },
    [MODULE_WIFI] = {
        .tag = "WIFI",
        .stack_size = 4096,
        .priority = 7,
    },
    [MODULE_ZIGBEE_ZBOSS] = {
        .tag = "ZIGBEE-ZBOSS",
        .stack_size = 4096,
        .priority = 5,
    },
    [MODULE_ZIGBEE_STATE] = {
        .tag = "ZIGBEE-STATE",
        .stack_size = 2048,
        .priority = 6,
    },
#ifdef IS_MONITOR_TASK_ENABLED
    [MODULE_MONITOR] = {
        .tag = "MONITOR",
        .stack_size = 2048,
        .priority = 3,
    }
#endif
};