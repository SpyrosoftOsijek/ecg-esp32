#include "mock_ecg_data_provider.h"

namespace ecg {
namespace provider {

void MockECGDataServiceProvider::AddTestData(const std::uint16_t* data, std::size_t length) {     
    AddECGData(data, length);
}

void MockECGDataServiceProvider::AddECGData(const std::uint16_t* data, std::size_t length) {
    for (std::size_t i = 0; i < length; i++) {
        dataset_.push(data[i]);
    }
}

bool MockECGDataServiceProvider::PollECGData() {
    while (!dataset_.empty()) {
        InvokeCallback(dataset_.front());
        dataset_.pop();
    }
    return true;
}

void MockECGDataServiceProvider::SetUpTestData(std::uint16_t data) {
    dataset_.push(data);
}


} // namespace provider
} // namespace ecg
