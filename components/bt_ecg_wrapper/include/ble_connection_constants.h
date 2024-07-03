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

static const char* BLE_ECG_TAG = "ESP32_SPP";
#define SPP_SERVER_NAME "ESP32_SPP_SERVER"
#define DEVICE_NAME "ESP32_S3"

#define PROFILE_APP_ID 0x55
#define GATTS_NUM_HANDLE_TEST_A 4
#define GATTS_CHAR_ONE 0xCCDD

#define adv_config_flag (1 << 0)
#define scan_rsp_config_flag (1 << 1)
