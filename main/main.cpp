#include "bluetooth_ecg_wrapper.h"
#include "esp_log.h"
#include "freertos/projdefs.h"
//#include "ecg.h"
//#include "uart_ecg_data_provider.h"


constexpr const char* MAIN_TAG = "Main";

extern "C" void app_main(void)
{
    ESP_LOGD(MAIN_TAG, "Application startup");

    // auto uartDataProvider = std::make_unique<ecg::provider::UartECGDataProvider>(CONFIG_EMULATE_UART_GPIO_TX, CONFIG_EMULATE_UART_GPIO_RX);
    // ecg::ECG ecg(std::move(uartDataProvider)); 


    BluetoothECGWrapper bt_ecg_wrapper{};
    std::uint16_t my_data{0x1234};

    while (1)
    {
        if (bt_ecg_wrapper.isConnected())
        {
            bt_ecg_wrapper.send_data(++my_data);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }

}
