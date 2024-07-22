#include <memory>
#include "esp_log.h"
#include "ecg.h"
#include "uart_ecg_data_provider.h"

constexpr const char* MAIN_TAG = "Main";
constexpr std::uint32_t kTaskStackSize{2048};
    constexpr unsigned int kTaskUxPriority{5};

extern "C" void app_main(void) {
    ESP_LOGD(MAIN_TAG, "Application startup");
    std::size_t mtu_data_size = 62; 
    ECGDataQueue data_queue;

    auto uartDataProvider = std::make_unique<ecg::provider::UartECGDataProvider>(CONFIG_EMULATE_UART_GPIO_TX, CONFIG_EMULATE_UART_GPIO_RX);
    ecg::ECG ecg(std::move(uartDataProvider), mtu_data_size); 
    data_queue= ecg.GetECGDataQueue();
   
    ecg.StartGatheringECGData(kTaskStackSize, kTaskUxPriority);
}
