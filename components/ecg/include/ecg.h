#ifndef ECG_ECG_H
#define ECG_ECG_H

#include "iecg_data_provider.h"
#include <memory>
namespace ecg{
using namespace provider;
class ECG {
public:
    ECG(std::unique_ptr<IECGDataProvider> ecg_data_provider_ptr, std::size_t mtu_data_size) noexcept;
    void StartGatheringECGData(std::uint32_t kTaskStackSize, unsigned int kTaskUxPriority);
    const ECGDataQueue& GetECGDataQueue() const;
    void DisplayECGData();
    
private:
    static void PollTask(void* pv_parameters);
    std::unique_ptr<IECGDataProvider> ecg_data_provider_;
    ECGDataQueue data_queue_;
    std::size_t mtu_data_size_;
};
}

#endif 
