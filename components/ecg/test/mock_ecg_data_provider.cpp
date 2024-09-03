#include "mock_ecg_data_provider.h"

namespace ecg {
namespace provider {

void MockECGDataProvider::AddTestData(const std::uint16_t* data, std::size_t size) {
    for (std::size_t i = 0; i < size; ++i) {
        mock_data_.push(data[i]);
    }
}

bool MockECGDataProvider::PollECGData() {
    if (!mock_data_.empty()) {
        InvokeCallback(mock_data_.front());
        mock_data_.pop();
    return true;
    }
    return false;
}

} // namespace provider
} // namespace ecg

