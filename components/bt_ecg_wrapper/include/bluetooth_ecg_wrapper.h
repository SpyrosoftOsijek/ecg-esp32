#ifndef BT_ECG_WRAPPER_BLUETOOTH_ECG_WRAPPER_H
#define BT_ECG_WRAPPER_BLUETOOTH_ECG_WRAPPER_H

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"

#include <cstdint>

class BluetoothECGWrapper
{

public:
    BluetoothECGWrapper();
    void send_data(std::uint16_t data);
    bool isConnected();
private:
    void bluetooth_initialize();  
    void bluetooth_setup();
    static void esp_gatts_cb(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
    static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
};

#endif // BT_ECG_WRAPPER_BLUETOOTH_ECG_WRAPPER_H
