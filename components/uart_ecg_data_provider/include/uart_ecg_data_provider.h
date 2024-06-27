#ifndef UART_ECG_DATA_PROVIDER_UARTECGDATAPROVIDER_H
#define UART_ECG_DATA_PROVIDER_UARTECGDATAPROVIDER_H

#include "soft_uart.h"
#include "iecg_data_provider.h"
#include <cstdint>
#include <queue>
#include <array>
#include <memory>

constexpr std::size_t kExpectedReadSize = 1024;
constexpr const char*  kUartEcgDataProviderTag = "uartECGDataProvider";
constexpr std::uint8_t kStartByte = 0xaa;
constexpr std::uint8_t kEndByte = 0xef;

namespace ecg
{
    
namespace provider{
class UartECGDataProvider : public IECGDataProvider
{
public:
    UartECGDataProvider(const std::uint32_t tx_pin, const std::uint32_t rx_pin);
    bool PollECGData() override;

#ifndef UNIT_TESTING
private:
#endif //UNIT_TESTING
    void initialize(const std::uint32_t tx_pin, const std::uint32_t rx_pin);
    void parseECGData();
    bool isPackageValid(const std::size_t end_index);
    void parseEcgPackage(const std::size_t start_index, const std::size_t end_index);

    std::unique_ptr<soft_uart_port_t> soft_uart_port_;
    std::array<std::uint8_t, kExpectedReadSize> receive_buffer_{};
};
}
using ecg_raw_data_array = std::array<std::uint8_t, kExpectedReadSize>;
}
#endif