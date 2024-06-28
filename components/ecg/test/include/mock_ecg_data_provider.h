#ifndef ECG_MOCK_ECG_DATA_PROVIDER_H
#define ECG_MOCK_ECG_DATA_PROVIDER_H

#include "iecg_data_provider.h"
#include <cstddef>
#include <cstdint>
#include <queue>
#include <vector>

namespace ecg {
namespace provider {

class MockECGDataServiceProvider : public IECGDataProvider {
public:
    MockECGDataServiceProvider() {}

    bool PollECGData() override;
    void AddTestData(const uint16_t* data, size_t length);
    void SetUpTestData(std::uint16_t data);

private:
    void AddECGData(const uint16_t* data, size_t length);
    std::queue<std::uint16_t> dataset_;
};
} // namespace provider
} // namespace ecg
#endif // MOCK_ECG_DATA_PROVIDER_H
