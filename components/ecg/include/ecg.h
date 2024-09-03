#ifndef ECG_ECG_H
#define ECG_ECG_H

#include "iecg_data_provider.h"
#include <memory>
namespace ecg{

using namespace provider;
class ECG {
public:
    ECG(std::unique_ptr<IECGDataProvider> ecg_data_provider_ptr, ECGDataCallback data_callback, std::size_t mtu_data_size) noexcept;

    void PollData() noexcept;

private:
    std::unique_ptr<IECGDataProvider> ecg_data_provider_;
    std::size_t mtu_data_size_;
};
}

#endif 

