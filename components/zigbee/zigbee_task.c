#include "zigbee_task.h"
#include "esp_zigbee.h"
#include "ezbee/zha.h"
#include "ezbee/zcl.h"
#include "ezbee/bdb.h"
#include "ezbee/zdo.h"
#include "ezbee/af.h"
#include "ezbee/core.h"
#include "ezbee/app_signals.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_random.h"
#include "esp_ieee802154.h"

#include "status_queue.h"
#include "net_queue.h"
#include "zigbee_init.h"
#include "zigbee_transport.h"
#include "netmap.h"
#include "system_events.h"

#define ZIGBEE_CHANNEL_MASK     0x07FFF800U
#define TAG                     g_module_tag[MODULE_ZIGBEE]

#define BRIGHTNESS_BASE_PCT     20
#define BRIGHTNESS_STEP_PCT     5

static uint8_t s_current_level = 25;

static void setup_all_local_states(void)
{
    s_current_level = 25;
    zigbee_update_brightness(1, 25, true);
    zigbee_update_on_off(1, true, true);
}

static void setup_all_spoofing(void)
{
    zigbee_apply_spoofing(1);
}

static bool app_signal_handler(const ezb_app_signal_t *signal) 
{
    ezb_app_signal_type_t sig_type = ezb_app_signal_get_type(signal);

    switch (sig_type) 
    {
        case EZB_ZDO_SIGNAL_SKIP_STARTUP:
            ESP_LOGI(TAG, "[SIGNAL] SKIP_STARTUP → BDB INITIALIZATION");
            ezb_bdb_start_top_level_commissioning(EZB_BDB_MODE_INITIALIZATION);
            break;

        case EZB_BDB_SIGNAL_DEVICE_FIRST_START:
        case EZB_BDB_SIGNAL_DEVICE_REBOOT: 
        {
            bool is_reboot = (sig_type == EZB_BDB_SIGNAL_DEVICE_REBOOT);
            const ezb_bdb_signal_simple_params_t *params = ezb_app_signal_get_params(signal);
            ESP_LOGI(TAG, "[SIGNAL] %s status=%d factory_new=%d",
                is_reboot ? "DEVICE_REBOOT" : "DEVICE_FIRST_START",
                params ? params->status : -1,
                ezb_bdb_is_factory_new());

            if (params && params->status == EZB_BDB_STATUS_SUCCESS) 
            {
                setup_all_spoofing();
                
                if (!ezb_bdb_is_factory_new()) 
                {
                    ESP_LOGI(TAG, "✓ Устройство уже в сети после перезагрузки. Синхронизируем состояние (0%)...");
                    setup_all_local_states();

                    system_events_set_bits(SYS_EVENT_ZIGBEE_CONNECTED_BIT);
                } 
                else 
                {
                    ESP_LOGI(TAG, "→ Запуск NETWORK_STEERING");
                    ezb_bdb_start_top_level_commissioning(EZB_BDB_MODE_NETWORK_STEERING);
                }
            } 
            else 
            {
                ESP_LOGW(TAG, "→ BDB init failed (status=%d), запуск NETWORK_STEERING", params ? params->status : -1);
                ezb_bdb_start_top_level_commissioning(EZB_BDB_MODE_NETWORK_STEERING);
            }
            break;
        }

        case EZB_BDB_SIGNAL_STEERING: 
        {
            const ezb_bdb_signal_simple_params_t *params = ezb_app_signal_get_params(signal);
            ESP_LOGI(TAG, "[SIGNAL] STEERING status=%d", params ? params->status : -1);
            if (params && params->status == EZB_BDB_STATUS_SUCCESS) 
            {
                setup_all_spoofing();
                
                ESP_LOGI(TAG, "✓ Подключено к сети! Синхронизируем состояние с хабом (0%)...");
                setup_all_local_states();

                system_events_set_bits(SYS_EVENT_ZIGBEE_CONNECTED_BIT);
            } 
            else 
            {
                ESP_LOGW(TAG, "✗ Сеть не найдена (status=%d). Повтор через 5с...", params ? params->status : -1);
                vTaskDelay(pdMS_TO_TICKS(5000));
                ezb_bdb_start_top_level_commissioning(EZB_BDB_MODE_NETWORK_STEERING);
            }
            break;
        }

        case EZB_ZDO_SIGNAL_LEAVE:
            ESP_LOGW(TAG, "[SIGNAL] LEAVE — удалён из сети координатором");
            system_events_clear_bits(SYS_EVENT_ZIGBEE_CONNECTED_BIT);
            break;

        case EZB_NWK_SIGNAL_PERMIT_JOIN_STATUS: 
        {
            const ezb_nwk_signal_permit_join_status_params_t *params = ezb_app_signal_get_params(signal);
            if (params && params->duration) 
            {
                ESP_LOGI(TAG, "[SIGNAL] PERMIT_JOIN: Сеть открыта на %d секунд. (PAN ID: 0x%04hx)", params->duration, ezb_nwk_get_panid());
            } 
            else 
            {
                ESP_LOGW(TAG, "[SIGNAL] PERMIT_JOIN: Сеть закрыта для новых устройств. (PAN ID: 0x%04hx)", ezb_nwk_get_panid());
            }
            break;
        }

        default:
            ESP_LOGI(TAG, "[SIGNAL] Unknown sig_type=0x%04X", (unsigned)sig_type);
            break;
    }
    return false;
}

static void process_level_command(uint8_t level)
{
    if (level == 0)
    {
        s_current_level = 0;
        ESP_LOGI(TAG, "Brightness level 0 (0%%) received, lamp turned OFF");
        zigbee_update_brightness(1, 0, false);
        zigbee_update_on_off(1, false, false);
        return;
    }

    // Convert Zigbee Level (25..254) to app percentage (1..100%).
    // Tuya/Sber hub maps 0..100% to 25..254 (level = 25 + pct * 229 / 100).
    uint8_t pct;
    if (level <= 25)
    {
        pct = 1;
    }
    else if (level >= 254)
    {
        pct = 100;
    }
    else
    {
        pct = (uint8_t)(((uint32_t)(level - 25) * 100 + 114) / 229);
        if (pct == 0)
        {
            pct = 1;
        }
    }

    if (pct < BRIGHTNESS_BASE_PCT)
    {
        ESP_LOGI(TAG, "Brightness %d%% (level %d) is below base %d%%, bulb state updated only",
                 pct, level, BRIGHTNESS_BASE_PCT);
    }
    else
    {
        uint8_t slot = (pct - BRIGHTNESS_BASE_PCT) / BRIGHTNESS_STEP_PCT;
        uint8_t sw_idx = slot / 2;
        bool state = (slot % 2 == 0); // slot 0, 2, 4... -> ENABLE; slot 1, 3, 5... -> DISABLE

        if (sw_idx < DEVS_COUNT)
        {
            uint8_t range_start = BRIGHTNESS_BASE_PCT + slot * BRIGHTNESS_STEP_PCT;
            uint8_t range_end = range_start + BRIGHTNESS_STEP_PCT - 1;

            ESP_LOGI(TAG, "Brightness %d%% (level %d) in range [%d..%d%%] -> Switch %d (node %d, tag %s): %s",
                     pct, level, range_start, range_end,
                     sw_idx + 1, zanni_virt_devs[sw_idx].node_id, zanni_virt_devs[sw_idx].tag,
                     state ? "ENABLE" : "DISABLE");

            static gate_msg_t gate_msg;
            static status_msg_t status_msg = 
            {
                .module = MODULE_ZIGBEE,
                .status = MODULE_STATUS_TX
            };

            gate_msg.node_id = zanni_virt_devs[sw_idx].node_id;
            gate_msg.payload.device_type = zanni_virt_devs[sw_idx].type;
            gate_msg.payload.status = state;
            snprintf(gate_msg.payload.tag, sizeof(gate_msg.payload.tag), "%.7s", zanni_virt_devs[sw_idx].tag);

            status_queue_send(status_msg, 0);
            net_queue_send(&gate_msg);
        }
        else
        {
            ESP_LOGI(TAG, "Brightness %d%% (level %d) beyond DEVS_COUNT=%d (switch %d), bulb state updated only",
                     pct, level, DEVS_COUNT, sw_idx + 1);
        }
    }

    uint8_t report_level = (uint8_t)(25 + ((uint32_t)pct * 229 + 50) / 100);
    if (report_level > 254)
    {
        report_level = 254;
    }

    s_current_level = report_level;

    zigbee_update_brightness(1, report_level, false);
    zigbee_update_on_off(1, true, false);
}

void zb_action_handler(ezb_zcl_core_action_callback_id_t callback_id, void *message) 
{
    switch (callback_id) 
    {
        case EZB_ZCL_CORE_SET_ATTR_VALUE_CB_ID: 
        {
            ezb_zcl_set_attr_value_message_t *msg = (ezb_zcl_set_attr_value_message_t *)message;
            if (!msg || msg->info.status != EZB_ZCL_STATUS_SUCCESS)
            {
                break;
            }
            msg->out.result = EZB_ZCL_STATUS_SUCCESS;

            if (msg->info.cluster_id == EZB_ZCL_CLUSTER_ID_LEVEL && 
                msg->in.attribute.id == EZB_ZCL_ATTR_LEVEL_CURRENT_LEVEL_ID) 
            {
                uint8_t level = *(uint8_t *)msg->in.attribute.data.value;

                if (level == s_current_level)
                {
                    ESP_LOGI(TAG, "Level %d unchanged (same as current %d), ignoring duplicate",
                             level, s_current_level);
                    break;
                }

                s_current_level = level;
                process_level_command(level);
            }
            else if (msg->info.cluster_id == EZB_ZCL_CLUSTER_ID_ON_OFF && 
                     msg->in.attribute.id == EZB_ZCL_ATTR_ON_OFF_ON_OFF_ID) 
            {
                bool on_off = *(bool *)msg->in.attribute.data.value;
                ESP_LOGI(TAG, "Bulb On/Off command received: %s", on_off ? "ON" : "OFF");
                zigbee_update_on_off(msg->info.dst_ep, on_off, false);
            }
            break;
        }

        default:
            ESP_LOGW(TAG, "Unhandled ZCL action callback_id: %d", callback_id);
            break;
    }
}

void zigbee_task(void *pvParameters) 
{
    ESP_LOGI(TAG, "Waiting for Wi-Fi connection before starting Zigbee...");
    system_events_wait_bits(SYS_EVENT_WIFI_CONNECTED_BIT, portMAX_DELAY);
    ESP_LOGI(TAG, "Wi-Fi connected. Starting Zigbee...");

    zanni_zigbee_init();

    // Set channels for search (11-26)
    ezb_bdb_set_primary_channel_set(ZIGBEE_CHANNEL_MASK); 

    // 1. Creating global device
    ezb_af_device_desc_t dev_desc = ezb_af_create_device_desc();
    
    zigbee_init_endpoints(dev_desc);
    
    ezb_af_device_desc_register(dev_desc);

    // 5. Регистрируем обработчики событий
    ezb_zcl_core_action_handler_register(zb_action_handler);
    ezb_app_signal_add_handler(app_signal_handler);

    // 7. Запускаем главный цикл ZBOSS
    esp_zigbee_start(false);
    esp_zigbee_launch_mainloop();
}