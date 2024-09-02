#include "mock_ecg_data_provider.h"
#include "ecg.h"
#include "unity.h"
#include <vector>

namespace ecg::provider {

std::vector<std::uint16_t> received_data; 

void test_callback(std::uint16_t data) {
    received_data.push_back(data);
}

TEST_CASE("Test MockECGDataProvider - Add and Poll Data", "[ECG]") {
    auto mock_provider = std::make_unique<MockECGDataProvider>();
    std::vector<std::uint16_t> test_data = {0x0191, 0xffb7, 0x0089, 0x0001};
    mock_provider->AddTestData(test_data.data(), test_data.size());
    ECG ecg(std::move(mock_provider), test_callback, 62);

    ecg.PollData();
    ecg.PollData();
    ecg.PollData();
    ecg.PollData();

    TEST_ASSERT_EQUAL(test_data.size(), received_data.size());
    for (std::size_t i = 0; i < test_data.size(); i++) {
        TEST_ASSERT_EQUAL(test_data[i], received_data[i]);
    }
    received_data.clear();
}

} // namespace ecg::provider
