#pragma once

#define GET_MODULE_FROM_CODE(code)          (code & 0xF000)
#define GET_STATUS_FROM_CODE(code)          (code & 0xF)

#define SET_MODULE_TO_CODE(module, code)    code |= (module << 12)
#define SET_STATUS_TO_CODE(module, code)    code |= (module)

typedef enum 
{
    MODULE_WIFI,
    MODULE_ZIGBEE,
    MODULE_MAX
} module_e;

static const char* const g_module_tag[] =
{
    [MODULE_WIFI] = "WIFI",
    [MODULE_ZIGBEE] = "ZIGBEE",
};

typedef enum 
{
    MODULE_STATUS_OK,
    MODULE_STATUS_CONF,
    MODULE_STATUS_ERROR,
    MODULE_STATUS_MAX,
} module_status_e;