#include "bluetooth_ecg_wrapper.h"

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

static const char *TAG = "ESP32_SPP";
#define SPP_SERVER_NAME "ESP32_SPP_SERVER"
#define DEVICE_NAME "ESP32_S3"

#define PROFILE_APP_ID 0x55
#define PROFILE_APP_IDX 0
#define SERVICE_UUID 0xAABB
#define GATTS_NUM_HANDLE_TEST_A 4
#define GATTS_CHAR_ONE 0xCCDD

#define adv_config_flag (1 << 0)
#define scan_rsp_config_flag (1 << 1)

static uint8_t adv_config_done = 0;

static uint8_t service_uuid[16] = {
    0xfb,
    0x34,
    0x9b,
    0x5f,
    0x80,
    0x00,
    0x00,
    0x80,
    0x00,
    0x10,
    0x00,
    0x00,
    0xFF,
    0x00,
    0x00,
    0x00,
};

static esp_bt_uuid_t char_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {
    .uuid128 = {
            0xfb,
            0x34,
            0x9b,
            0x5f,
            0x80,
            0x00,
            0x00,
            0x80,
            0x00,
            0x10,
            0x00,
            0x00,
            0xAA,
            0x00,
            0x00,
            0x00,
            }
    },
};

static esp_ble_adv_data_t adv_data = {
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
    .service_uuid_len = sizeof(service_uuid),
    .p_service_uuid = service_uuid,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp = true,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x20,
    .max_interval = 0x40,
    .appearance = 0x00,
    .manufacturer_len = 0,       // TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = NULL, //&test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = 16,
    .p_service_uuid = service_uuid,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_scan_params_t ble_scan_params = {
    .scan_type = BLE_SCAN_TYPE_ACTIVE,
    .own_addr_type = BLE_ADDR_TYPE_RPA_PUBLIC,
    .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval = 0x50,
    .scan_window = 0x30,
    .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE,
};

static esp_ble_adv_params_t adv_params = {
    .adv_int_min = 0x20,
    .adv_int_max = 0x40,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    //.peer_addr        =
    //.peer_addr_type   =
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static gatts_profile_inst gl_profile_tab{};

BluetoothECGWrapper::BluetoothECGWrapper()
{
    bluetooth_initialize();
    bluetooth_setup();
    gl_profile_tab.gatts_cb = esp_gatts_cb;
    gl_profile_tab.gatts_if = ESP_GATT_IF_NONE;
    gl_profile_tab.char_uuid = char_uuid;
}

void BluetoothECGWrapper::send_data(uint8_t data)
{
     while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        esp_ble_gatts_set_attr_value(gl_profile_tab.char_handle, sizeof(data), &data);
        esp_ble_gatts_send_indicate(gl_profile_tab.gatts_if, gl_profile_tab.conn_id, gl_profile_tab.char_handle, sizeof(data), &data, true);
    }
    
}

void BluetoothECGWrapper::esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~adv_config_flag);
        if (adv_config_done == 0)
        {
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
    default:
        break;
    }
}

void BluetoothECGWrapper::esp_gatts_cb(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event)
    {
    default:
        break;
    case ESP_GATTS_REG_EVT:
        esp_ble_gap_set_device_name(DEVICE_NAME);
        if (param->reg.status == ESP_GATT_OK)
        {
            gl_profile_tab.gatts_if = gatts_if;
        }
        esp_ble_gap_config_adv_data(&adv_data);
        gl_profile_tab.service_id.is_primary = true;
        gl_profile_tab.service_id.id.inst_id = 0x00;
        gl_profile_tab.service_id.id.uuid.len = sizeof(service_uuid);
        memcpy(gl_profile_tab.service_id.id.uuid.uuid.uuid128, &service_uuid, ESP_UUID_LEN_128);
        esp_ble_gatts_create_service(gatts_if, &gl_profile_tab.service_id, GATTS_NUM_HANDLE_TEST_A);
        break;
    case ESP_GATTS_CREATE_EVT:{
        gl_profile_tab.service_handle = param->create.service_handle;
        gl_profile_tab.char_uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab.char_uuid.uuid.uuid16 = GATTS_CHAR_ONE;

        esp_gatt_char_prop_t a_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
        uint8_t value = 10;
        uint8_t *char_ptr = &value;
        esp_attr_control_t rsp_mode {ESP_GATT_AUTO_RSP};
        esp_attr_value_t gatts_char1_value = {10,10,char_ptr};
        esp_err_t add_char_ret =
        esp_ble_gatts_add_char(gl_profile_tab.service_handle,
                                &gl_profile_tab.char_uuid,
                                ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                a_property,
                                &gatts_char1_value,
                                &rsp_mode);

        esp_ble_gatts_start_service(gl_profile_tab.service_handle);
        break;
        }

    case ESP_GATTS_ADD_CHAR_EVT:
     {
        gl_profile_tab.char_handle = param->add_char.attr_handle;
        break;
    }
    
    case ESP_GATTS_CONNECT_EVT:{
        esp_ble_conn_update_params_t conn_params = {0};
        memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
        conn_params.latency = 0;
        conn_params.max_int = 0x30; // max_int = 0x30*1.25ms = 40ms
        conn_params.min_int = 0x10; // min_int = 0x10*1.25ms = 20ms
        conn_params.timeout = 400;  // timeout = 400*10ms = 4000ms
        gl_profile_tab.conn_id = param->connect.conn_id;
        esp_ble_gap_update_conn_params(&conn_params);
        break;
     }

    case ESP_GATTS_DISCONNECT_EVT:{
        esp_ble_gap_start_advertising(&adv_params);
        esp_ble_gap_config_adv_data(&scan_rsp_data);
        adv_config_done |= scan_rsp_config_flag;
        break;
    }
    }
}

void BluetoothECGWrapper::bluetooth_initialize()
{
    nvs_flash_init();
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_bt_controller_init(&bt_cfg);
    esp_bt_controller_enable(ESP_BT_MODE_BLE);
    esp_bluedroid_init();
    esp_bluedroid_enable();
}

void BluetoothECGWrapper::bluetooth_setup()
{
    esp_ble_gatts_register_callback(esp_gatts_cb);
    esp_ble_gap_register_callback(esp_gap_cb);
    esp_ble_gatts_app_register(PROFILE_APP_ID);
    esp_ble_gatt_set_local_mtu(250);
}