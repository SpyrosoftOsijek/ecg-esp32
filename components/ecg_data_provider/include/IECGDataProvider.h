#ifndef IECG_DATA_PROVIDER_H
#define IECG_DATA_PROVIDER_H

#include <cstddef>
#include <cstdint>
#include <queue>
#include <functional>
#include "esp_err.h" 


using ECGDataQueue = std::queue<std::uint16_t>;
using ECGDataCallback = std::function<void(const std::uint16_t)>;

namespace ecgData{

class IECGDataProvider {
public:
    virtual  bool pollECGData()=0;
    virtual void setCallback(ECGDataCallback cb) {
        callback = cb;
    }
    virtual ECGDataQueue& getECGQueue() {
        return dataQueue;
    }

protected:
    ECGDataQueue dataQueue{};
    virtual void invokeCallback( uint16_t data) {
        if (callback) {
            callback(data);
        }
    }
    friend class ECG; 
    ECGDataCallback callback = nullptr;

};
}
#endif
