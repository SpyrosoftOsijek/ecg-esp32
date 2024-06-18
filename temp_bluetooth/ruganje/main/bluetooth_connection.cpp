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
#include "heart_rate_table.h"

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

struct gatts_profile_inst
{
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
};

static uint8_t service_uuid[16] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    // first uuid, 16bit, [12],[13] is the value
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

/* The length of adv data must be less than 31 bytes */
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

// scan response data
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

static void gatts_profile_a_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);


static struct gatts_profile_inst gl_profile_tab[1] = {
    [PROFILE_APP_IDX] = {
        .gatts_cb = gatts_profile_a_event_handler,
        .gatts_if = ESP_GATT_IF_NONE,
    },
};

static const uint16_t GATTS_SERVICE_UUID_TEST = 0x00FF;
static const uint16_t CHAR_1_SHORT_WR = 0xFF01;
static const uint16_t CHAR_2_LONG_WR = 0xFF02;
static const uint16_t CHAR_3_SHORT_NOTIFY = 0xFF03;

static const uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
static const uint16_t character_user_description = ESP_GATT_UUID_CHAR_DESCRIPTION;
static const uint8_t char_prop_notify = ESP_GATT_CHAR_PROP_BIT_NOTIFY;
static const uint8_t char_prop_read_write = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_READ;
static const uint8_t char1_name[] = "Char_1_Short_WR";
static const uint8_t char2_name[] = "Char_2_Long_WR";
static const uint8_t char3_name[] = "Char_3_Short_Notify";
static const uint8_t char_ccc[2] = {0x00, 0x00};
static const uint8_t char_value[4] = {0x11, 0x22, 0x33, 0x44};

static const esp_gatts_attr_db_t gatt_db[HRS_IDX_NB] =
    {

};

static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
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

static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    ESP_LOGI(TAG, "EVT %d, gattc if %d", event, gattc_if);

    if (event == ESP_GATTC_REG_EVT)
    {
        if (param->reg.status == ESP_GATT_OK)
        {
            // ESP_LOGI(TAG, "GATT client registered successfully, gattc_if = %d", gattc_if);
        }
        else
        {
            // ESP_LOGI(TAG, "Reg app failed, status %d", param->reg.status);
            return;
        }
    }
}

static void gatts_profile_a_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event)
    {
    default:
        break;

    case ESP_GATTS_REG_EVT:
        ESP_LOGI(TAG, "REGISTER_APP_EVT, status %d, app_id %d", param->reg.status, param->reg.app_id);

        // Set up the service ID for Profile A
        gl_profile_tab[PROFILE_APP_IDX].service_id.is_primary = true;
        gl_profile_tab[PROFILE_APP_IDX].service_id.id.inst_id = 0x00;
        gl_profile_tab[PROFILE_APP_IDX].service_id.id.uuid.len = sizeof(service_uuid);
        memcpy(gl_profile_tab[PROFILE_APP_IDX].service_id.id.uuid.uuid.uuid128, &service_uuid, ESP_UUID_LEN_128);
        esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[PROFILE_APP_IDX].service_id, GATTS_NUM_HANDLE_TEST_A);
        break;

    case ESP_GATTS_CREATE_EVT:
        ESP_LOGI(TAG, "CREATE_SERVICE_EVT, status %d, service_handle %d", param->create.status, param->create.service_handle);
        gl_profile_tab[PROFILE_APP_IDX].service_handle = param->create.service_handle;
        gl_profile_tab[PROFILE_APP_IDX].char_uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PROFILE_APP_IDX].char_uuid.uuid.uuid16 = GATTS_CHAR_ONE;

        esp_ble_gatts_start_service(gl_profile_tab[PROFILE_APP_IDX].service_handle);
        esp_gatt_char_prop_t a_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
        uint8_t value = 10;
        uint8_t *char_ptr = &value;
        esp_attr_control_t rsp_mode {ESP_GATT_RSP_BY_APP};
        esp_attr_value_t gatts_char1_value = {10,10,char_ptr};
        esp_err_t add_char_ret =
        esp_ble_gatts_add_char(gl_profile_tab[PROFILE_APP_IDX].service_handle,
                                &gl_profile_tab[PROFILE_APP_IDX].char_uuid,
                                ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                a_property,
                                &gatts_char1_value,
                                &rsp_mode);
        break;
    }
}

static void esp_gatts_cb(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event)
    {
    default:
        break;
    case ESP_GATTS_REG_EVT:
        esp_ble_gap_set_device_name(DEVICE_NAME);
        // ESP_LOGI(TAG, "REGISTER_APP_EVT, status %d, app_id %d", param->reg.status, param->reg.app_id);
        if (param->reg.status == ESP_GATT_OK)
        {
            gl_profile_tab[param->reg.app_id].gatts_if = gatts_if;
        }
        esp_ble_gap_config_adv_data(&adv_data);
        break;

    case ESP_GATTS_CREATE_EVT:
        ESP_LOGI(TAG, "CREATE_SERVICE_EVT, status %d, service_handle %d", param->create.status, param->create.service_handle);
        break;

    case ESP_GATTS_CONNECT_EVT:
    {
        esp_ble_conn_update_params_t conn_params = {0};
        memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
        conn_params.latency = 0;
        conn_params.max_int = 0x30; // max_int = 0x30*1.25ms = 40ms
        conn_params.min_int = 0x10; // min_int = 0x10*1.25ms = 20ms
        conn_params.timeout = 400;  // timeout = 400*10ms = 4000ms
        // ESP_LOGI(TAG, "ESP_GATTS_CONNECT_EVT, conn_id %d, remote %02x:%02x:%02x:%02x:%02x:%02x:, is_conn %d",
        //          param->connect.conn_id,
        //          param->connect.remote_bda[0],
        //          param->connect.remote_bda[1],
        //          param->connect.remote_bda[2],
        //          param->connect.remote_bda[3],
        //          param->connect.remote_bda[4],
        //          param->connect.remote_bda[5]);
        gl_profile_tab[PROFILE_APP_IDX].conn_id = param->connect.conn_id;
        esp_ble_gap_update_conn_params(&conn_params);
        uint8_t data[2] = {0xAA,0xBB};
        esp_ble_gatts_send_indicate(gatts_if,
                                                gl_profile_tab[PROFILE_APP_IDX].conn_id,
                                                gl_profile_tab[PROFILE_APP_IDX].char_handle,
                                                sizeof(data)-1,
                                                data,
                                                true);
    }   

    case ESP_GATTS_READ_EVT:
    {
        // ESP_LOGI(TAG, "GATT_READ_EVT, conn_id %d, trans_id %d, handle %d",
        //           param->read.conn_id, param->read.trans_id, param->read.handle);
        // esp_gatt_rsp_t rsp;
        // memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
        // rsp.attr_value.handle = param->read.handle;
        // rsp.attr_value.len = 4;
        // rsp.attr_value.value[0] = 0xde;
        // rsp.attr_value.value[1] = 0xed;
        // rsp.attr_value.value[2] = 0xbe;
        // rsp.attr_value.value[3] = 0xef;
        // esp_ble_gatts_send_response(gatts_if,
        //                             param->read.conn_id,
        //                             param->read.trans_id,
        //                             ESP_GATT_OK, &rsp);
    }
    break;

    case ESP_GATTS_EXEC_WRITE_EVT:
        // ESP_LOGI(TAG, "GATT_WRITE_EVT, value len %d, value :", param->write.len);
        break;
    // case ESP_GATTS_WRITE_EVT:
    // {
    //     // ESP_LOGI(TAG, "GATT_WRITE_EVT, conn_id %d, trans_id %d, handle %u", param->write.conn_id, param->write.trans_id, param->write.handle);
    //     if (!param->write.is_prep)
    //     {
    //         // ESP_LOGI(TAG, "GATT_WRITE_EVT, value len %d, value :", param->write.len);
    //         esp_log_buffer_hex(TAG, param->write.value, param->write.len); 

    //         uint16_t descr_value = param->write.value[1] << 8 | param->write.value[0];
    //         if (descr_value == 0x0001)
    //         {
    //             // Assuming notify is enabled
    //             ESP_LOGI(TAG, "notify enable");
    //             // Create sample notify data
    //             uint8_t notify_data[15];
    //             for (int i = 0; i < sizeof(notify_data); ++i)
    //             {
    //                 notify_data[i] = i % 0xff;
    //             }
    //             // Send indication
    //             esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id,
    //                                         param->write.handle,
    //                                         sizeof(notify_data),
    //                                         notify_data, false);
    //         }
    //         else if (descr_value == 0x0002)
    //         {
    //             // Assuming indicate is enabled
    //             ESP_LOGI(TAG, "indicate enable");
    //             // Create sample indicate data
    //             uint8_t indicate_data[15];
    //             for (int i = 0; i < sizeof(indicate_data); ++i)
    //             {
    //                 indicate_data[i] = i % 0xff;
    //             }
    //             // Send indication
    //             esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id,
    //                                         param->write.handle,
    //                                         sizeof(indicate_data),
    //                                         indicate_data, true);
    //         }
    //         else if (descr_value == 0x0000)
    //         {
    //             // ESP_LOGI(TAG, "notify/indicate disable ");
    //         }
    //         else
    //         {
    //             // ESP_LOGE(TAG, "unknown value");
    //         }
    //     }
    // }
    // break;
    case ESP_GATTS_DISCONNECT_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_DISCONNECT_EVT, reason = %d", param->disconnect.reason);
        esp_ble_gap_start_advertising(&adv_params);
        break;

        esp_err_t ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
        if (ret)
        {
            // ESP_LOGE(TAG, "config scan response data failed, error code = %x", ret);
        }
        adv_config_done |= scan_rsp_config_flag;
    }
}

extern "C" void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret)
    {
        ESP_LOGE(TAG, "%s initialize controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret)
    {
        ESP_LOGE(TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }
    ret = esp_bluedroid_init();
    if (ret)
    {
        ESP_LOGE(TAG, "%s init bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return;
    }
    ret = esp_bluedroid_enable();
    if (ret)
    {
        ESP_LOGE(TAG, "%s enable bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return;
    }
    ret = esp_ble_gatts_register_callback(esp_gatts_cb);
    if (ret)
    {
        ESP_LOGE(TAG, "gatts register error, error code = %x", ret);
        return;
    }
    ret = esp_ble_gap_register_callback(esp_gap_cb);
    if (ret)
    {
        ESP_LOGE(TAG, "%s gap register error, error code = %x", __func__, ret);
        return;
    }
    ret = esp_ble_gatts_app_register(PROFILE_APP_ID);
    if (ret)
    {
        ESP_LOGE(TAG, "gatts app register error, error code = %x", ret);
        return;
    }
    esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(33);
    if (local_mtu_ret)
    {
        ESP_LOGE(TAG, "set local  MTU failed, error code = %x", local_mtu_ret);
    }
    ret = esp_ble_gattc_register_callback(esp_gattc_cb);
    if (ret)
    {
        ESP_LOGE(TAG, "%s gattc register error, error code = %x", __func__, ret);
        return;
    }
    ret = esp_ble_gattc_app_register(PROFILE_APP_ID);
    if (ret)
    {
        ESP_LOGE(TAG, "%s gattc app register error, error code = %x", __func__, ret);
    }
    ret = esp_ble_gatt_set_local_mtu(250);
    if (ret)
    {
        ESP_LOGE(TAG, "set local  MTU failed, error code = %x", ret);
    }
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}