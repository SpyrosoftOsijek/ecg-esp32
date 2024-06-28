// Include guards?
// const, noexcept metode?

#include <array>
#include <cstdint>
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

class BluetoothECGWrapper {

public:
    BluetoothECGWrapper();
    static BluetoothECGWrapper create();
    void send_data(const uint8_t data);

private:
    void bluetooth_initialize(); 
    void bluetooth_setup();
    void initialize();
    static void esp_gatts_cb(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
    static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
};
