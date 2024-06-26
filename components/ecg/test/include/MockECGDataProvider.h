#ifndef MOCK_ECG_DATA_PROVIDER_H
#define MOCK_ECG_DATA_PROVIDER_H

#include "IECGDataProvider.h"
#include <cstddef>
#include <cstdint>
#include <queue>

namespace ecgData {
    class MockECGDataServiceProvider : public IECGDataProvider {
    public:
        MockECGDataServiceProvider() {}
        bool pollECGData() {
            return true;
        }
        void addTestData(const uint16_t* data, size_t length);
        std::queue<uint16_t>& getECGQueue();

    private:
        void addECGData(const uint16_t* data, size_t length);
        std::queue<uint16_t> dataQueue;
    };
}

#endif 
