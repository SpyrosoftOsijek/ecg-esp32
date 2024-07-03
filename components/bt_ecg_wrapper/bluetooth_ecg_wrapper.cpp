#include "bluetooth_ecg_wrapper.h"
#include "uuid_helper.h"

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

constexpr const char *BLE_ECG_TAG = "ESP32_SPP";

bool bt_initialized{false};
bool bt_setup_done{false};
bool connected{false};

uint16_t conn_id{0};
uint16_t gatts_if{ESP_GATT_IF_NONE};

static constexpr std::uint16_t PROFILE_APP_ID = 0x1234;
static constexpr std::uint16_t GATTS_NUM_HANDLE = 4;

static uuid::uuid128_t SERVICE_UUID { uuid::to_byte_array("16695c8c-ef83-4e58-9688-020537829756") };
constexpr std::uint8_t SERVICE_INSTANCE_ID = 0; // 0 if unused
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

static esp_gatt_srvc_id_t &get_service_id()
{
    static esp_gatt_srvc_id_t service_id{};
    service_id.is_primary = true;
    service_id.id.inst_id = SERVICE_INSTANCE_ID;
    service_id.id.uuid.len = ESP_UUID_LEN_128;
    std::copy(SERVICE_UUID.cbegin(), SERVICE_UUID.cend(), service_id.id.uuid.uuid.uuid128);
    return service_id;
}

void start_service_and_add_char(std::uint16_t service_handle)
{
    esp_bt_uuid_t characteristic_uuid{.len = ESP_UUID_LEN_128};
    auto char_uuid128 = uuid::to_byte_array("8b117e00-e0b6-4567-b32f-09e7b852dde4");
    memcpy(characteristic_uuid.uuid.uuid128, char_uuid128.data(), ESP_UUID_LEN_128);
    esp_attr_control_t control{.auto_rsp = ESP_GATT_AUTO_RSP};

    std::uint16_t char_uint16_value = 0;
    uint8_t *char_ptr = reinterpret_cast<uint8_t *>(&char_uint16_value);
    esp_attr_value_t char_uint16 = {
        .attr_max_len = 10,
        .attr_len = sizeof(char_uint16_value),
        .attr_value = char_ptr,
    };

    esp_err_t add_char_ret = esp_ble_gatts_add_char(service_handle,
                                                    &characteristic_uuid,
                                                    ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                                    ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY | ESP_GATT_CHAR_PROP_BIT_INDICATE,
                                                    &char_uint16,
                                                    &control);

    if (add_char_ret != ESP_OK)
    {
        ESP_LOGE(BLE_ECG_TAG, "Add char failed, error code = %x", add_char_ret);
        return;
    }

    esp_err_t start_service_ret = esp_ble_gatts_start_service(service_handle);
    if (start_service_ret != ESP_OK)
    {
        ESP_LOGE(BLE_ECG_TAG, "Start service failed, error code = %x", start_service_ret);
        return;
    }

    ESP_LOGI(BLE_ECG_TAG, "Service started, characteristic added");
}

void setup_service(esp_gatt_if_t local_gatts_if)
{
    gatts_if = local_gatts_if;
    ESP_ERROR_CHECK(esp_ble_gap_config_adv_data(&adv_data));
    ESP_ERROR_CHECK(esp_ble_gatts_create_service(gatts_if, &get_service_id(), GATTS_NUM_HANDLE));
}

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
        if (esp_ble_gap_start_advertising(&adv_params) != ESP_OK)
        {
            ESP_LOGE(BLE_ECG_TAG, "Failed to start advertising");
        }
        break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
        connected = true;
        break;
    default:
        break;
    }
}

void BluetoothECGWrapper::esp_gatts_cb(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GATTS_REG_EVT:
    {
        setup_service(gatts_if);
        break;

        ESP_LOGI(BLE_ECG_TAG, "ESP_GATTS_REG_EVT");
        if (!connected)
        {
            if (auto set_dev_name_ret = esp_ble_gap_set_device_name("HEHAD"); ESP_OK != set_dev_name_ret)
            {
                ESP_LOGE(BLE_ECG_TAG, "set device name failed, error code = %x", set_dev_name_ret);
            }
            setup_service(gatts_if);
        }
        break;
    }
    case ESP_GATTS_CREATE_EVT:
    {
        ESP_LOGI(BLE_ECG_TAG, "ESP_GATTS_CREATE_EVT triggered");
        ESP_LOGI(BLE_ECG_TAG, "status: %d, service_handle %d, uuid: %s",
                 param->create.status, param->create.service_handle, uuid::to_string(param->create.service_id.id.uuid.uuid.uuid128).c_str());
        service_handle = param->create.service_handle;
        start_service_and_add_char(service_handle);
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
        ESP_LOGI(BLE_ECG_TAG, "ESP_GATTS_DISCONNECT_EVT triggered");
        connected = false;
        setup_service(gatts_if);
        break;
    }
    default:
        break;
    }
}
