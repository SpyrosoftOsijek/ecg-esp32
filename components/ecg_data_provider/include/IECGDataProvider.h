#ifndef IECG_DATA_PROVIDER_H
#define IECG_DATA_PROVIDER_H

#include <cstddef>
#include <cstdint>
#include <queue>

using ECGDataQueue = std::queue<std::uint16_t>;

class IECGDataProvider {
public:
    virtual void parseECGData() = 0;
    virtual ECGDataQueue& getECGQueue() {
        return dataQueue;
    }
protected:
    virtual void addECGData(const uint16_t* data, size_t length) = 0;

    ECGDataQueue dataQueue{};
};

#endif
