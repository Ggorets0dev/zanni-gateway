#pragma once

typedef enum 
{
    MODULE_STATUS,
    MODULE_WIFI,
    MODULE_ZIGBEE,
    MODULE_MAX
} module_e;

static const char* const g_module_tag[] =
{
    [MODULE_STATUS] = "STATUS",
    [MODULE_WIFI] = "WIFI",
    [MODULE_ZIGBEE] = "ZIGBEE",
};