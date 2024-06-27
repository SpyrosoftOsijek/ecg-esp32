#include "mock_ecg_data_provider.h"
#include "ECG.h"
#include "unity.h"
#include <queue>
#include <vector>
using namespace ecg;
using namespace ecg::provider;

TEST_CASE("Test adding data to ECG queue-neww", "[ECG]") {
    auto mock_provider = std::make_unique<MockECGDataServiceProvider>();
    std::vector<std::uint16_t> test_data = {0x0191, 0xffb7, 0x0089, 0x0001};
    mock_provider->AddTestData(test_data.data(), test_data.size());
    ECG ecg(std::move(mock_provider));

    ecg.StartGatheringECGData();
    std::queue<std::uint16_t> data_queue = ecg.GetECGDataQueue();

    for (std::size_t i = 0; i < test_data.size(); i++){
        TEST_ASSERT_EQUAL(test_data[i], data_queue.front());
        data_queue.pop();
    }
}