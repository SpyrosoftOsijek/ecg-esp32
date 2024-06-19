#include "ECG.h"
#include "esp_log.h"

ECG::ECG( IECGDataProvider& serviceProviderRef) noexcept
    : serviceProvider(serviceProviderRef), dataQueue{serviceProviderRef.getECGQueue()} {}



const ECGDataQueue& ECG::getECGDataQueue() const {
 const ECGDataQueue& ecgQueue = serviceProvider.getECGQueue();
    return ecgQueue;
 }

 
void ECG::displayECGData() {
    while (!dataQueue.empty()) {
        uint16_t ECGData = dataQueue.front();
        dataQueue.pop();
        ESP_LOGI("Display ECG data", "ECG Data: 0x%04x", ECGData);
    }
}
