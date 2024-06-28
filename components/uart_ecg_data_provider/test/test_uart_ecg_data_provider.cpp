#include "uart_ecg_data_provider.h"
#include "unity.h"
#include <vector>

TEST_CASE("Test deserializing raw data received from uart", "[ecg_uart_data_provider]")
{
    constexpr std::uint8_t kRawDataLength = 0x04;
    constexpr std::array<std::uint16_t, kRawDataLength> kExpectedDeserializedData{0x0191, 0xffb7, 0x0089, 0x0001};

    ecg::provider::UartECGDataProvider provider{CONFIG_EMULATE_UART_GPIO_TX, CONFIG_EMULATE_UART_GPIO_RX};
    provider.receive_buffer_ = {kStartByte, kRawDataLength, 0x01, 0x91, 0xff, 0xb7, 0x00, 0x89, 0x00, 0x01, kEndByte};

    std::vector<std::uint16_t> out_data{};
    provider.SetCallback([&out_data](const auto parsed_data_element)
                         { out_data.push_back(parsed_data_element); });

    provider.parseECGData();

    TEST_ASSERT_EQUAL(kExpectedDeserializedData.size(), out_data.size());
    for (std::size_t i = 0; i < out_data.size(); ++i)
    {
        TEST_ASSERT_EQUAL(kExpectedDeserializedData[i], out_data[i]);
    }
}
