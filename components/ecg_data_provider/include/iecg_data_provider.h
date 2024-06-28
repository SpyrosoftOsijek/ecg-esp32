#ifndef ECG_DATA_PROVIDER_IECG_DATA_PROVIDER_H
#define ECG_DATA_PROVIDER_IECG_DATA_PROVIDER_H

#include <cstdint>
#include <functional>
#include<queue>

using ECGDataQueue = std::queue<std::uint16_t>;
using ECGDataCallback = std::function<void(const std::uint16_t)>;

namespace ecg
{
namespace provider{
class IECGDataProvider
{
public:
    virtual bool PollECGData() = 0;
    virtual void SetCallback(ECGDataCallback cb)
    {
        callback_ = cb;
    }
    virtual ECGDataQueue& GetECGQueue()
    {
        return data_queue_;
    }

protected:
    virtual void InvokeCallback(uint16_t data)
    {
        if (callback_)
        {
            callback_(data);
        }
    }
    
    ECGDataQueue data_queue_{};
    ECGDataCallback callback_ = nullptr;
};  
} //namespace provider
} // namespace ecg
#endif // IECG_DATA_PROVIDER_H