#include <memory>
#include "ecg.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

constexpr char* kEcgTag = "ECG";
constexpr TaskHandle_t* const kTaskPXCreatedTask{nullptr};

namespace ecg
{

ECG::ECG(std::unique_ptr<IECGDataProvider> ecg_data_provider_ptr, std::size_t mtu_data_size) noexcept
    : ecg_data_provider_(std::move(ecg_data_provider_ptr)), mtu_data_size_(mtu_data_size) {
        data_queue_ = ecg_data_provider_->GetECGQueue();
        ecg_data_provider_->SetCallback([this](const std::uint16_t data) {
            if (this->data_queue_.size() < this->mtu_data_size_) {
                this->data_queue_.push(data);
            }    
        }
    );
}

void ECG::StartGatheringECGData(std::uint32_t kTaskStackSize, unsigned int kTaskUxPriority)
{
    xTaskCreate(PollTask, "ECGPollTask", kTaskStackSize, this, kTaskUxPriority, kTaskPXCreatedTask);
}

const ECGDataQueue& ECG::GetECGDataQueue() const
{
    return data_queue_;
}

void ECG::DisplayECGData()
{
    while (!data_queue_.empty())
    {
        std::uint16_t ECGData = data_queue_.front();
        data_queue_.pop();
        ESP_LOGD(kEcgTag, "ECG Data: 0x%04x", ECGData);
    }
}

void ECG::PollTask(void* pv_parameters)
{
    ECG* ecg = static_cast<ECG* >(pv_parameters);
    while (true)
    {
        ecg->ecg_data_provider_->PollECGData();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
} //namespace ecg
