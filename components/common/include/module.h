#pragma once

typedef enum 
{
    MODULE_WIFI,
    MODULE_ZIGBEE,
    MODULE_MAX
} module_e;

typedef enum 
{
    MODULE_STATUS_OK,
    MODULE_STATUS_CONF,
    MODULE_STATUS_ERROR,
    MODULE_STATUS_MAX,
} module_status_e;

static const char* const g_module_tag[] =
{
    [MODULE_WIFI] = "WIFI",
    [MODULE_ZIGBEE] = "ZIGBEE",
};