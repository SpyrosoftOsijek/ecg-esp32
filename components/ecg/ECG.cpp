#include "ECG.h"
#include "esp_log.h"
const char* ECG_TAG = "ECG";
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

ECG::ECG( IECGDataProvider& ecgDataProviderRef) noexcept
    : ecgDataProvider(ecgDataProviderRef), dataQueue{ecgDataProviderRef.getECGQueue()} {

     ecgDataProvider.setCallback([this](std::uint16_t data) {
        this->dataQueue.push(data);
        this->displayECGData();       
    });
    }

const ECGDataQueue& ECG::getECGDataQueue() const {
    return dataQueue;
 }
 
void ECG::displayECGData() {
    while (!dataQueue.empty()) {
        uint16_t ECGData = dataQueue.front();
        dataQueue.pop();
        ESP_LOGI(ECG_TAG, "ECG Data: 0x%04x", ECGData);
    }
}

void ECG::startGatheringECGData() {
    xTaskCreate(pollTask, "ECGPollTask", 2048, this, 5, nullptr);
}

void ECG::pollTask(void* arg) {
    ECG* ecg = static_cast<ECG*>(arg);
    while (true) {
        ecg->ecgDataProvider.pollECGData();
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}


