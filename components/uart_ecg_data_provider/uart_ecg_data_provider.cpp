#include "uart_ecg_data_provider.h"
#include "esp_log.h"

namespace ecg
{
namespace provider{
UartECGDataProvider::UartECGDataProvider(const std::uint32_t tx_pin, const std::uint32_t rx_pin) : soft_uart_port_(std::make_unique<soft_uart_port_t>())
{
    initialize(tx_pin, rx_pin);
}

bool UartECGDataProvider::PollECGData()
{
    esp_err_t ret = soft_uart_receive(*soft_uart_port_, receive_buffer_.data(), kExpectedReadSize);
    if (ret == ESP_OK)
    {
        parseECGData();
        return true;
    }
    else{
        ESP_LOGE(kUartEcgDataProviderTag, "Failed to receive UART data");
        return false; 
    }
}

void UartECGDataProvider::initialize(const std::uint32_t tx_pin, const std::uint32_t rx_pin)
{
    soft_uart_config_t config = {
        .tx_pin = tx_pin,
        .rx_pin = rx_pin,
        .baudrate = SOFT_UART_115200};

    esp_err_t ret = soft_uart_new(&config, soft_uart_port_.get());
    ESP_ERROR_CHECK(ret);
}

void UartECGDataProvider::parseECGData()
{
    for (std::size_t i = 0; i < receive_buffer_.max_size(); ++i)
    {
        if (kStartByte == receive_buffer_[i])
        {
            const auto data_length = receive_buffer_[i + 1];
            const auto end_package_data_idx = i + 2 + (data_length * 2);

            if (isPackageValid(end_package_data_idx))
            {
                const auto start_package_data_idx = i + 2;
                parseEcgPackage(start_package_data_idx, end_package_data_idx);
                i = end_package_data_idx + 1;
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
    for (std::size_t i = start_index; i < end_index; i += 2)
    {
        const std::uint16_t ecg_data_element = (receive_buffer_[i] << 8) + receive_buffer_[i + 1];
        ESP_LOGD(kUartEcgDataProviderTag, "Deserialized data element: %u", ecg_data_element);
        InvokeCallback(ecg_data_element);
    }
}
} //namespace provider
} // namespace ecg