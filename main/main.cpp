#include "bluetooth_ecg_wrapper.h"
#include "esp_log.h"
#include "freertos/projdefs.h"
#include "ecg.h"
#include "uart_ecg_data_provider.h"
#include <memory>

constexpr const char *MAIN_TAG = "Main";

extern "C" void app_main(void)
{

    ESP_LOGD(MAIN_TAG, "Application startup");
    std::size_t mtu_data_size = 62;

    BluetoothECGWrapper bt_ecg_wrapper{};

    auto data_received_callback = [&bt_ecg_wrapper](auto data)
    {
        ESP_LOGI(MAIN_TAG, "Sending data: %d", data);
        bt_ecg_wrapper.send_data(data);
    };

    auto uartDataProvider = std::make_unique<ecg::provider::UartECGDataProvider>(CONFIG_EMULATE_UART_GPIO_TX, CONFIG_EMULATE_UART_GPIO_RX);
    ecg::ECG ecg(std::move(uartDataProvider), data_received_callback, mtu_data_size);

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
        if (bt_ecg_wrapper.isConnected())
        {
            ecg.PollData();
        }
    }

}
