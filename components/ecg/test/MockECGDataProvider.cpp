#include "MockECGDataProvider.h"

namespace ecgData {

    void MockECGDataServiceProvider::addTestData(const uint16_t* data, size_t length) {
        addECGData(data, length);
    }

    void MockECGDataServiceProvider::addECGData(const uint16_t* data, size_t length) {
        for (size_t i = 0; i < length; i++) {
            dataQueue.push(data[i]);
        }
    }

    std::queue<uint16_t>& MockECGDataServiceProvider::getECGQueue() {
        return dataQueue;
    }
}
