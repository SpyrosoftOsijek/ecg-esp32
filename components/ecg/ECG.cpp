#include "ECG.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

const char *ECG_TAG = "ECG"; 
constexpr std::uint32_t TASK_STACK_SIZE{2048};
constexpr unsigned int TASK_UX_PRIORITY{5};
constexpr TaskHandle_t * const TASK__PX_CREATED_TASK{nullptr};

namespace ecgData{
    
ECG::ECG(IECGDataProvider &ecgDataProviderRef) noexcept
    : ecgDataProvider(ecgDataProviderRef), dataQueue{ecgDataProviderRef.getECGQueue()}
{
    ecgDataProvider.setCallback([this](std::uint16_t data)
                                { this->dataQueue.push(data); });
}

const ECGDataQueue &ECG::getECGDataQueue() const
{
    return dataQueue;
}

void ECG::displayECGData()
{
    while (!dataQueue.empty())
    {
        uint16_t ECGData = dataQueue.front();
        dataQueue.pop();
        ESP_LOGI(ECG_TAG, "ECG Data: 0x%04x", ECGData);
    }
}

void ECG::startGatheringECGData()
{
    xTaskCreate(pollTask, "ECGPollTask", TASK_STACK_SIZE, this, TASK_UX_PRIORITY, TASK__PX_CREATED_TASK);
}

void ECG::pollTask(void *pvParameters )
{
    ECG *ecg = static_cast<ECG *>(pvParameters);
    while (true)
    {
        ecg->ecgDataProvider.pollECGData();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
}