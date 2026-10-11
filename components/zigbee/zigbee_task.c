#include "zigbee_task.h"
#include "esp_zigbee.h"
#include "ezbee/zha.h"
#include "ezbee/zcl.h"
#include "ezbee/bdb.h"
#include "ezbee/zdo.h"
#include "ezbee/af.h"
#include "ezbee/core.h"
#include "esp_log.h"
#include "ezbee/app_signals.h"
#include "esp_random.h"
#include "esp_ieee802154.h"

#include "status_queue.h"
#include "net_queue.h"
#include "zigbee_init.h"
#include "netmap.h"
#include "system_events.h"

#define TAG "ZIGBEE"

// Spoofing: мимикрируем под SONOFF BASICZBR3
// ZCL Character String: первый байт = длина строки (без нуля)
static char s_manuf_name[] = "\x06""SONOFF";
static char s_model_id[]   = "\x09""BASICZBR3";

#define SONOFF_ENDPOINT 1


static void apply_sonoff_spoofing(void)
{
    esp_err_t r1 = ezb_zcl_set_attr_value(
        SONOFF_ENDPOINT,
        EZB_ZCL_CLUSTER_ID_BASIC,
        EZB_ZCL_CLUSTER_SERVER,
        EZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID,
        EZB_ZCL_STD_MANUF_CODE,
        s_manuf_name,
        false
    );
    esp_err_t r2 = ezb_zcl_set_attr_value(
        SONOFF_ENDPOINT,
        EZB_ZCL_CLUSTER_ID_BASIC,
        EZB_ZCL_CLUSTER_SERVER,
        EZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID,
        EZB_ZCL_STD_MANUF_CODE,
        s_model_id,
        false
    );
    ESP_LOGI(TAG, "Spoofing: ManufName=%s ModelID=%s (r1=%d r2=%d)",
        s_manuf_name + 1, s_model_id + 1, r1, r2);
}

static void update_local_state_and_report(uint8_t endpoint, bool new_state)
{
    ezb_zcl_set_attr_value(
        endpoint,
        EZB_ZCL_CLUSTER_ID_ON_OFF,
        EZB_ZCL_CLUSTER_SERVER,
        EZB_ZCL_ATTR_ON_OFF_ON_OFF_ID,
        EZB_ZCL_STD_MANUF_CODE,
        &new_state,
        false
    );

    ezb_zcl_report_attr_cmd_t report_cmd = {
        .cmd_ctrl = {
            .dst_addr = EZB_ADDRESS_SHORT(0x0000), // Явно указываем адрес хаба (координатора)
            .dst_ep = 1,                           // Эндпоинт хаба
            .src_ep = endpoint,
            .cluster_id = EZB_ZCL_CLUSTER_ID_ON_OFF,
            .manuf_code = EZB_ZCL_STD_MANUF_CODE,
            .fc = {
                .manuf_specific = 0,
                .direction = EZB_ZCL_CMD_DIRECTION_TO_CLI,
                .dis_default_rsp = 0
            }
        },
        .payload = {
            .attr_id = EZB_ZCL_ATTR_ON_OFF_ON_OFF_ID
        }
    };
    ezb_zcl_report_attr_cmd_req(&report_cmd);
}

static bool app_signal_handler(const ezb_app_signal_t *signal) 
{
    ezb_app_signal_type_t sig_type = ezb_app_signal_get_type(signal);

    switch (sig_type) {
        case EZB_ZDO_SIGNAL_SKIP_STARTUP:
            ESP_LOGI(TAG, "[SIGNAL] SKIP_STARTUP → BDB INITIALIZATION");
            ezb_bdb_start_top_level_commissioning(EZB_BDB_MODE_INITIALIZATION);
            break;

        case EZB_BDB_SIGNAL_DEVICE_FIRST_START:
        case EZB_BDB_SIGNAL_DEVICE_REBOOT: {
            bool is_reboot = (sig_type == EZB_BDB_SIGNAL_DEVICE_REBOOT);
            const ezb_bdb_signal_simple_params_t *params = ezb_app_signal_get_params(signal);
            ESP_LOGI(TAG, "[SIGNAL] %s status=%d factory_new=%d",
                is_reboot ? "DEVICE_REBOOT" : "DEVICE_FIRST_START",
                params ? params->status : -1,
                ezb_bdb_is_factory_new());

            if (params && params->status == EZB_BDB_STATUS_SUCCESS) {
                apply_sonoff_spoofing();
                if (!ezb_bdb_is_factory_new()) {
                    ESP_LOGI(TAG, "✓ Устройство уже в сети после перезагрузки. Синхронизируем состояние (ВЫКЛ)...");
                    update_local_state_and_report(SONOFF_ENDPOINT, false);

                    system_events_set_bits(SYS_EVENT_ZIGBEE_CONNECTED_BIT);
                } else {
                    ESP_LOGI(TAG, "→ Запуск NETWORK_STEERING");
                    ezb_bdb_start_top_level_commissioning(EZB_BDB_MODE_NETWORK_STEERING);
                }
            } else {
                ESP_LOGW(TAG, "→ BDB init failed (status=%d), запуск NETWORK_STEERING", params ? params->status : -1);
                ezb_bdb_start_top_level_commissioning(EZB_BDB_MODE_NETWORK_STEERING);
            }
            break;
        }

        case EZB_BDB_SIGNAL_STEERING: {
            const ezb_bdb_signal_simple_params_t *params = ezb_app_signal_get_params(signal);
            ESP_LOGI(TAG, "[SIGNAL] STEERING status=%d", params ? params->status : -1);
            if (params && params->status == EZB_BDB_STATUS_SUCCESS) {
                apply_sonoff_spoofing();
                ESP_LOGI(TAG, "✓ Подключено к сети! Синхронизируем состояние с хабом (ВЫКЛ)...");
                update_local_state_and_report(SONOFF_ENDPOINT, false);

                system_events_set_bits(SYS_EVENT_ZIGBEE_CONNECTED_BIT);
            } else {
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

        case EZB_NWK_SIGNAL_PERMIT_JOIN_STATUS: {
            const ezb_nwk_signal_permit_join_status_params_t *params = ezb_app_signal_get_params(signal);
            if (params && params->duration) {
                ESP_LOGI(TAG, "[SIGNAL] PERMIT_JOIN: Сеть открыта на %d секунд. (PAN ID: 0x%04hx)", params->duration, ezb_nwk_get_panid());
            } else {
                ESP_LOGW(TAG, "[SIGNAL] PERMIT_JOIN: Сеть закрыта для новых устройств. (PAN ID: 0x%04hx)", ezb_nwk_get_panid());
            }
            break;
        }

        default:
            // Логируем ВСЕ неизвестные сигналы — помогает понять что делает хаб после join
            ESP_LOGI(TAG, "[SIGNAL] Unknown sig_type=0x%04X", (unsigned)sig_type);
            break;
    }
    return false;
}

void zb_action_handler(ezb_zcl_core_action_callback_id_t callback_id, void *message) {
    static gate_msg_t gate_msg;
    
    static status_msg_t status_msg = 
    {
        .module = MODULE_ZIGBEE,
        .status = MODULE_STATUS_TX
    };

    switch (callback_id) {
        case EZB_ZCL_CORE_SET_ATTR_VALUE_CB_ID: {
            ezb_zcl_set_attr_value_message_t *msg = (ezb_zcl_set_attr_value_message_t *)message;
            
            // Подтверждаем стеку успешную обработку атрибута
            msg->out.result = EZB_ZCL_STATUS_SUCCESS;

            if (msg->info.cluster_id == EZB_ZCL_CLUSTER_ID_ON_OFF && 
                msg->in.attribute.id == EZB_ZCL_ATTR_ON_OFF_ON_OFF_ID) {
                
                bool state = *(bool *)msg->in.attribute.data.value;
                
                ESP_LOGI("ZIGBEE", "Получена команда для Endpoint %d: %s", 
                         msg->info.dst_ep, 
                         state ? "ВКЛЮЧИТЬ" : "ВЫКЛЮЧИТЬ");
                
                // FIXME: Hardcoded values
                gate_msg.node_id = zanni_virt_devs[0].node_id;
                gate_msg.payload.device_type = zanni_virt_devs[0].type;
                gate_msg.payload.status = state;
                snprintf(gate_msg.payload.tag, sizeof(gate_msg.payload.tag), "%s", zanni_virt_devs[0].tag);

                status_queue_send(status_msg, 0);
                net_queue_send(&gate_msg);

                // Принудительно отправляем Report обратно хабу (Координатору)
                ezb_zcl_report_attr_cmd_t report_cmd = {
                    .cmd_ctrl = {
                        .dst_addr = EZB_ADDRESS_SHORT(0x0000), // Явно указываем адрес хаба
                        .dst_ep = 1,                           // Эндпоинт хаба (обычно 1)
                        .src_ep = msg->info.dst_ep,
                        .cluster_id = EZB_ZCL_CLUSTER_ID_ON_OFF,
                        .manuf_code = EZB_ZCL_STD_MANUF_CODE,
                        .fc = {
                            .manuf_specific = 0,
                            .direction = EZB_ZCL_CMD_DIRECTION_TO_CLI,
                            .dis_default_rsp = 0
                        }
                    },
                    .payload = {
                        .attr_id = EZB_ZCL_ATTR_ON_OFF_ON_OFF_ID
                    }
                };
                ezb_zcl_report_attr_cmd_req(&report_cmd);
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

    // Разрешить поиск сети на всех стандартных каналах (11-26)
    ezb_bdb_set_primary_channel_set(0x07FFF800U); 

    // 1. Создаем устройство EZBee и конфигурацию On/Off Light
    ezb_af_device_desc_t dev_desc = ezb_af_create_device_desc();
    ezb_zha_on_off_light_config_t light_cfg = EZB_ZHA_ON_OFF_LIGHT_CONFIG();
    
    // 2. Создаем Endpoint (канал реле)
    uint8_t endpoint = 1;
    ezb_af_ep_desc_t ep_desc = ezb_zha_create_on_off_light(endpoint, &light_cfg);

    // 3. Добавляем кастомные атрибуты (Spoofing) в Basic Cluster
    //    Используем те же строки что и apply_sonoff_spoofing(), для консистентности.
    //    apply_sonoff_spoofing() будет вызвана повторно после старта стека — это гарантирует
    //    что NVS не перетрёт наши значения.
    ezb_zcl_cluster_desc_t basic_cluster = ezb_af_endpoint_get_cluster_desc(ep_desc, EZB_ZCL_CLUSTER_ID_BASIC, EZB_ZCL_CLUSTER_SERVER);
    if (basic_cluster) {
        ezb_zcl_basic_cluster_desc_add_attr(basic_cluster, EZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID, s_manuf_name);
        ezb_zcl_basic_cluster_desc_add_attr(basic_cluster, EZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID, s_model_id);
    }

    // 4. Привязываем Endpoint к устройству и регистрируем в стеке
    ezb_af_device_add_endpoint_desc(dev_desc, ep_desc);
    ezb_af_device_desc_register(dev_desc);

    // 5. Регистрируем обработчики событий
    ezb_zcl_core_action_handler_register(zb_action_handler);
    ezb_app_signal_add_handler(app_signal_handler);

    // 7. Запускаем главный цикл ZBOSS
    esp_zigbee_start(false);
    esp_zigbee_launch_mainloop();
}