#include "bluetooth_ecg_wrapper.h"
#include "esp_log.h"
#include "ecg.h"
#include "uart_ecg_data_provider.h"

#include <memory>

constexpr const char* MAIN_TAG = "Main";

extern "C" void app_main(void) {
    ESP_LOGD(MAIN_TAG, "Application startup");

    auto uartDataProvider = std::make_unique<ecg::provider::UartECGDataProvider>(CONFIG_EMULATE_UART_GPIO_TX, CONFIG_EMULATE_UART_GPIO_RX);
    ecg::ECG ecg(std::move(uartDataProvider)); 

    ecg.StartGatheringECGData();

    BluetoothECGWrapper bt_ecg_wrapper{};
    uint16_t dummy_data = 5;
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
        bt_ecg_wrapper.send_data(dummy_data);
        dummy_data++;
    }
}
