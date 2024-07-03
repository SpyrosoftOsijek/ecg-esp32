#include "bluetooth_ecg_wrapper.h"
#include "ble_connection_constants.h"

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_bt.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"
#include <esp_gap_ble_api.h>
#include <nvs_flash.h>
#include <esp_gattc_api.h>
#include <esp_gatt_common_api.h>
#include <esp_gatts_api.h>
#include <string.h>
#include <cstdint>

bool bt_initialized{false};
bool bt_setup_done{false};
bool connected{false};

uint16_t conn_id{0};
uint16_t gatts_if{ESP_GATT_IF_NONE};

uint8_t service_uuid[16] = {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00};
uint16_t service_handle{0};

esp_bt_uuid_t ecg_char_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {
        .uuid128 = {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xAA, 0x00, 0x00, 0x00}},
};
uint16_t ecg_char_handle{0};

static uint8_t advertizing_service_uuid[16] = {0xfb, 0x34, 0x9a, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00};
static esp_ble_adv_data_t adv_data{
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x20,
    .max_interval = 0x40,
    .appearance = 0x00,
    .manufacturer_len = 0,       // TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = NULL, // test_manufacturer,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = ESP_UUID_LEN_128,
    .p_service_uuid = advertizing_service_uuid,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_params_t adv_params = {
    .adv_int_min = 0x20,
    .adv_int_max = 0x40,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .peer_addr = {},
    .peer_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

BluetoothECGWrapper::BluetoothECGWrapper()
{
    bluetooth_initialize();
    bluetooth_setup();
}

void BluetoothECGWrapper::bluetooth_initialize()
{
    if (!bt_initialized)
    {
        nvs_flash_init();
        esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
        esp_bt_controller_init(&bt_cfg);
        esp_bt_controller_enable(ESP_BT_MODE_BLE);
        esp_bluedroid_init();
        esp_bluedroid_enable();
        bt_initialized = true;
    }
}

void BluetoothECGWrapper::bluetooth_setup()
{
    if (!bt_setup_done)
    {
        esp_ble_gatts_register_callback(esp_gatts_cb);
        esp_ble_gap_register_callback(esp_gap_cb);
        esp_ble_gatts_app_register(PROFILE_APP_ID);
        esp_ble_gatt_set_local_mtu(250);
        bt_setup_done = true;
    }
}

void BluetoothECGWrapper::send_data(std::uint16_t data)
{
    std::uint8_t *serialized_data = reinterpret_cast<std::uint8_t *>(&data);
    if (connected)
    {
        esp_ble_gatts_set_attr_value(ecg_char_handle, sizeof(data), serialized_data);
        esp_ble_gatts_send_indicate(gatts_if, conn_id, ecg_char_handle, sizeof(data), serialized_data, true);
    }
    else
    {
        ESP_LOGE(BLE_ECG_TAG, "Not connected, cannot send data!");
    }
}

void BluetoothECGWrapper::esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_SCAN_RESULT_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_SCAN_RESULT_EVT");
        break;
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_ADV_START_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_SCAN_START_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_AUTH_CMPL_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_AUTH_CMPL_EVT");
        break;
    case ESP_GAP_BLE_KEY_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_KEY_EVT");
        break;
    case ESP_GAP_BLE_SEC_REQ_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_SEC_REQ_EVT");
        break;
    case ESP_GAP_BLE_PASSKEY_NOTIF_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_PASSKEY_NOTIF_EVT");
        break;
    case ESP_GAP_BLE_PASSKEY_REQ_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_PASSKEY_REQ_EVT");
        break;
    case ESP_GAP_BLE_OOB_REQ_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_OOB_REQ_EVT");
        break;
    case ESP_GAP_BLE_LOCAL_IR_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_LOCAL_IR_EVT");
        break;
    case ESP_GAP_BLE_LOCAL_ER_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_LOCAL_ER_EVT");
        break;
    case ESP_GAP_BLE_NC_REQ_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_NC_REQ_EVT");
        break;
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_SET_STATIC_RAND_ADDR_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_SET_STATIC_RAND_ADDR_EVT");
        break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT");
        break;
    case ESP_GAP_BLE_SET_PKT_LENGTH_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_SET_PKT_LENGTH_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_SET_LOCAL_PRIVACY_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_SET_LOCAL_PRIVACY_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_CLEAR_BOND_DEV_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_CLEAR_BOND_DEV_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_GET_BOND_DEV_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_GET_BOND_DEV_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_READ_RSSI_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_READ_RSSI_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_UPDATE_WHITELIST_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_UPDATE_WHITELIST_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_UPDATE_DUPLICATE_EXCEPTIONAL_LIST_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_UPDATE_DUPLICATE_EXCEPTIONAL_LIST_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_SET_CHANNELS_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_SET_CHANNELS_EVT");
        break;
    case ESP_GAP_BLE_READ_PHY_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_READ_PHY_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_SET_PREFERRED_DEFAULT_PHY_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_SET_PREFERRED_DEFAULT_PHY_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_SET_PREFERRED_PHY_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_SET_PREFERRED_PHY_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_EXT_ADV_SET_RAND_ADDR_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_EXT_ADV_SET_RAND_ADDR_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_EXT_ADV_SET_PARAMS_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_EXT_ADV_SET_PARAMS_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_EXT_ADV_DATA_SET_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_EXT_ADV_DATA_SET_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_EXT_SCAN_RSP_DATA_SET_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_EXT_SCAN_RSP_DATA_SET_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_EXT_ADV_START_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_EXT_ADV_START_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_EXT_ADV_STOP_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_EXT_ADV_STOP_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_EXT_ADV_SET_REMOVE_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_EXT_ADV_SET_REMOVE_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_EXT_ADV_SET_CLEAR_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_EXT_ADV_SET_CLEAR_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_PERIODIC_ADV_SET_PARAMS_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_PERIODIC_ADV_SET_PARAMS_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_PERIODIC_ADV_DATA_SET_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_PERIODIC_ADV_DATA_SET_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_PERIODIC_ADV_START_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_PERIODIC_ADV_START_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_PERIODIC_ADV_STOP_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_PERIODIC_ADV_STOP_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_PERIODIC_ADV_CREATE_SYNC_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_PERIODIC_ADV_CREATE_SYNC_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_PERIODIC_ADV_SYNC_CANCEL_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_PERIODIC_ADV_SYNC_CANCEL_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_PERIODIC_ADV_SYNC_TERMINATE_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_PERIODIC_ADV_SYNC_TERMINATE_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_PERIODIC_ADV_ADD_DEV_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_PERIODIC_ADV_ADD_DEV_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_PERIODIC_ADV_REMOVE_DEV_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_PERIODIC_ADV_REMOVE_DEV_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_PERIODIC_ADV_CLEAR_DEV_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_PERIODIC_ADV_CLEAR_DEV_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_SET_EXT_SCAN_PARAMS_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_SET_EXT_SCAN_PARAMS_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_EXT_SCAN_START_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_EXT_SCAN_START_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_EXT_SCAN_STOP_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_EXT_SCAN_STOP_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_PREFER_EXT_CONN_PARAMS_SET_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_PREFER_EXT_CONN_PARAMS_SET_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_PHY_UPDATE_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_PHY_UPDATE_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_EXT_ADV_REPORT_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_EXT_ADV_REPORT_EVT");
        break;
    case ESP_GAP_BLE_SCAN_TIMEOUT_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_SCAN_TIMEOUT_EVT");
        break;
    case ESP_GAP_BLE_ADV_TERMINATED_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_ADV_TERMINATED_EVT");
        break;
    case ESP_GAP_BLE_SCAN_REQ_RECEIVED_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_SCAN_REQ_RECEIVED_EVT");
        break;
    case ESP_GAP_BLE_CHANNEL_SELECT_ALGORITHM_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_CHANNEL_SELECT_ALGORITHM_EVT");
        break;
    case ESP_GAP_BLE_PERIODIC_ADV_REPORT_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_PERIODIC_ADV_REPORT_EVT");
        break;
    case ESP_GAP_BLE_PERIODIC_ADV_SYNC_LOST_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_PERIODIC_ADV_SYNC_LOST_EVT");
        break;
    case ESP_GAP_BLE_PERIODIC_ADV_SYNC_ESTAB_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_PERIODIC_ADV_SYNC_ESTAB_EVT");
        break;
    case ESP_GAP_BLE_SC_OOB_REQ_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_SC_OOB_REQ_EVT");
        break;
    case ESP_GAP_BLE_SC_CR_LOC_OOB_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_SC_CR_LOC_OOB_EVT");
        break;
    case ESP_GAP_BLE_GET_DEV_NAME_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_GET_DEV_NAME_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_PERIODIC_ADV_RECV_ENABLE_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_PERIODIC_ADV_RECV_ENABLE_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_PERIODIC_ADV_SYNC_TRANS_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_PERIODIC_ADV_SYNC_TRANS_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_PERIODIC_ADV_SET_INFO_TRANS_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_PERIODIC_ADV_SET_INFO_TRANS_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_SET_PAST_PARAMS_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_SET_PAST_PARAMS_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_PERIODIC_ADV_SYNC_TRANS_RECV_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_PERIODIC_ADV_SYNC_TRANS_RECV_EVT");
        break;
    case ESP_GAP_BLE_DTM_TEST_UPDATE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_DTM_TEST_UPDATE_EVT");
        break;
    case ESP_GAP_BLE_ADV_CLEAR_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_ADV_CLEAR_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_VENDOR_CMD_COMPLETE_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_VENDOR_CMD_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_EVT_MAX:
        ESP_LOGI(BLE_ECG_TAG, "BLE_ECG_TESP_GAP_BLE_EVT_MAXAG");
        break;
    } // debug switch

    switch (event)
    {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
    {
        esp_ble_gap_start_advertising(&adv_params);
        ESP_LOGI(BLE_ECG_TAG, "Start advertising");
    }
    break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
        ESP_LOGI(BLE_ECG_TAG, "ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT triggered");
        connected = true;
        break;
    default:
        break;
    }
}

void BluetoothECGWrapper::esp_gatts_cb(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    ESP_LOGI(BLE_ECG_TAG, "esp_gatts_cb with gatts_if %d", gatts_if);
    switch (event)
    {
    case ESP_GATTS_REG_EVT:
    {
        ESP_LOGI(BLE_ECG_TAG, "ESP_GATTS_REG_EVT");
        if (!connected)
        {
            esp_ble_gap_set_device_name(DEVICE_NAME);
            esp_ble_gap_config_adv_data(&adv_data);
        }
        if (param->reg.status == ESP_GATT_OK)
        {
            ESP_LOGI(BLE_ECG_TAG, "gatts_if set to %d", gatts_if);
            gatts_if = gatts_if;
        }
        else
        {
            ESP_LOGE(BLE_ECG_TAG, "NISAM U REDU!");
        }

        esp_gatt_srvc_id_t service_id{};
        service_id.is_primary = true;
        service_id.id.inst_id = 0x00;
        // Generate uuid
        service_id.id.uuid.len = sizeof(service_uuid);
        memcpy(service_id.id.uuid.uuid.uuid128, &service_uuid, ESP_UUID_LEN_128);
        esp_ble_gatts_create_service(gatts_if, &service_id, GATTS_NUM_HANDLE_TEST_A);
        break;
    }
    case ESP_GATTS_CREATE_EVT:
    {
        uint16_t char_value{0};
        uint8_t *char_ptr = reinterpret_cast<uint8_t *>(&char_value);
        esp_attr_value_t gatts_char1_value = {2, 2, char_ptr};
        esp_attr_control_t rsp_mode{ESP_GATT_AUTO_RSP};
        esp_gatt_char_prop_t a_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY;

        ESP_LOGI(BLE_ECG_TAG, "ESP_GATTS_CREATE_EVT");
        service_handle = param->create.service_handle;
        esp_ble_gatts_add_char(service_handle,
                               &ecg_char_uuid,
                               ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                               a_property,
                               &gatts_char1_value,
                               &rsp_mode);

        esp_ble_gatts_start_service(service_handle);
        break;
    }

    case ESP_GATTS_ADD_CHAR_EVT:
    {
        ESP_LOGI(BLE_ECG_TAG, "ESP_GATTS_ADD_CHAR_EVT");
        ecg_char_handle = param->add_char.attr_handle;
        break;
    }

    case ESP_GATTS_CONNECT_EVT:
    {
        ESP_LOGI(BLE_ECG_TAG, "ESP_GATTS_CONNECT_EVT, conn_id %d, remote %02x:%02x:%02x:%02x:%02x:%02x:",
                 param->connect.conn_id,
                 param->connect.remote_bda[0], param->connect.remote_bda[1], param->connect.remote_bda[2],
                 param->connect.remote_bda[3], param->connect.remote_bda[4], param->connect.remote_bda[5]);
        conn_id = param->connect.conn_id;
        ESP_LOGI(BLE_ECG_TAG, "SET conn_id %d", param->connect.conn_id);
        break;
    }

    case ESP_GATTS_DISCONNECT_EVT:
    {
        ESP_LOGI(BLE_ECG_TAG, "ESP_GATTS_DISCONNECT_EVT");
        connected = false;
        esp_ble_gap_start_advertising(&adv_params);
        esp_ble_gap_config_adv_data(&adv_data);
        break;
    }
    default:
        break;
    }
}
