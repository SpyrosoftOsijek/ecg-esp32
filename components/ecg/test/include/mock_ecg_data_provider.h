#ifndef MOCK_ECG_DATA_PROVIDER_H
#define MOCK_ECG_DATA_PROVIDER_H

#include "iecg_data_provider.h"
#include <vector>
#include <queue>

namespace ecg {
namespace provider {

class MockECGDataProvider : public IECGDataProvider {
public:
    void AddTestData(const std::uint16_t* data, std::size_t size);
    bool PollECGData() override;

private:
    std::queue<std::uint16_t> mock_data_;
};

} // namespace provider
} // namespace ecg

#endif // MOCK_ECG_DATA_PROVIDER_H

