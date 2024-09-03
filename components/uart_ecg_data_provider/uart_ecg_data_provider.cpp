#include "uart_ecg_data_provider.h"
#include "esp_log.h"

namespace ecg {
namespace provider {

UartECGDataProvider::UartECGDataProvider(const std::uint32_t tx_pin, const std::uint32_t rx_pin)
{
    initialize(tx_pin, rx_pin);
    ESP_LOGE(kUartEcgDataProviderTag, "UartECGDataProvider initialized");
}

void UartECGDataProvider::initialize(const std::uint32_t tx_pin, const std::uint32_t rx_pin)
{
    soft_uart_config_t config = {
        .tx_pin = tx_pin,
        .rx_pin = rx_pin,
        .baudrate = SOFT_UART_115200
    };

    const auto ret = soft_uart_new(&config, &soft_uart_port_);

    if (ESP_OK != ret) {
        ESP_LOGE(kUartEcgDataProviderTag, "Failed to create soft uart port");
    }
}

UartECGDataProvider::~UartECGDataProvider()
{
    if (nullptr != soft_uart_port_) {
        soft_uart_del(soft_uart_port_);
    }
}

bool UartECGDataProvider::PollECGData()
{
    const auto ret = soft_uart_receive(soft_uart_port_, receive_buffer_.data(), receive_buffer_.size());
    if (ret == ESP_OK) {
        ESP_LOGI(kUartEcgDataProviderTag, "Start parsing");
        parseECGData();
        return true;
    }
    ESP_LOGE(kUartEcgDataProviderTag, "Failed to receive UART data");
    return false;
}

void UartECGDataProvider::parseECGData()
{
    for (std::size_t i = 0; i < receive_buffer_.max_size(); ++i) {
        ESP_LOGD(kUartEcgDataProviderTag, "data [%d]: %d", i, receive_buffer_[i]);
        if (kStartByte == receive_buffer_[i]) {
            ESP_LOGI(kUartEcgDataProviderTag, "Starting to parse");

            const auto data_length = receive_buffer_[i + 1];
            const auto end_package_data_idx = i + 2 + (data_length * 2);

            if (isPackageValid(end_package_data_idx)) {
                const auto start_package_data_idx = i + 2;
                parseEcgPackage(start_package_data_idx, end_package_data_idx);
                i = end_package_data_idx + 1;
            } else {
                ESP_LOGW(kUartEcgDataProviderTag, "Package invalid");
            }
        }
    }
}

bool UartECGDataProvider::isPackageValid(const std::size_t end_idx)
{
    return end_idx < kExpectedReadSize && kEndByte == receive_buffer_[end_idx];
}

void UartECGDataProvider::parseEcgPackage(const std::size_t start_index, const std::size_t end_index)
{
    for (std::size_t i = start_index; i < end_index; i += 2) {
        const std::uint16_t ecg_data_element = (receive_buffer_[i] << 8) + receive_buffer_[i + 1];
        ESP_LOGI(kUartEcgDataProviderTag, "Deserialized data element: %u", ecg_data_element);
        InvokeCallback(ecg_data_element);
    }
}

} // namespace provider
} // namespace ecg