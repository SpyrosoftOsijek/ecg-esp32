#ifndef IECG_DATA_PROVIDER_H
#define IECG_DATA_PROVIDER_H
#include <cstddef>
#include <cstdint>
#include <queue>
#include <functional>

#include "esp_err.h" 

using ECGDataQueue = std::queue<std::uint16_t>;
using ECGDataCallback = std::function<void(const ECGDataQueue&)>;


class IECGDataProvider {
public:
    virtual ECGDataQueue& getECGQueue() {
        return dataQueue;
    }

     virtual void setCallback(ECGDataCallback cb) {
        callback = cb;
    }

protected:
    ECGDataQueue dataQueue{};
    virtual  esp_err_t pollECGData() =0; 

    virtual void invokeCallback( uint16_t data) {
        if (callback) {
            dataQueue.push(data);
            callback(dataQueue);
        }
    }
    friend class ECG; 
    ECGDataCallback callback = nullptr;

};
#endif
