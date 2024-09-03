#include <memory>
#include "ecg.h"
#include "esp_log.h"

constexpr const char* kEcgTag = "ECG";

namespace ecg
{
    
ECG::ECG(std::unique_ptr<IECGDataProvider> ecg_data_provider_ptr, ECGDataCallback callback, std::size_t mtu_data_size) noexcept
    : ecg_data_provider_(std::move(ecg_data_provider_ptr)), mtu_data_size_(mtu_data_size) {
        ecg_data_provider_->SetCallback(callback);
}

void ECG::PollData() noexcept {
    ecg_data_provider_->PollECGData();
}

} //namespace ecg