#include "ECG.h"
#include "esp_log.h"

 //const char* EXAMPLE_TAG = "UARTManager";

void ECG::addECGData(const uint16_t* data, size_t length) noexcept {
    for (size_t i = 0; i < length; i++) {
        ECGQueue.push(data[i]);
        //ESP_LOGI("UARTManager", "Data added to queue: 0x%04x", data[i]);
    }
    flushFlag = 1;
}

void ECG::displayData() noexcept {
    std::queue<uint16_t> tempQueue = ECGQueue;

    if (flushFlag != 1) {
        return;
    } 
    flushFlag = 0;

    while (!tempQueue.empty()) {
        uint16_t ECGData = tempQueue.front();
        tempQueue.pop();
        ESP_LOGI("UARTManager", "ECG Data: 0x%04x", ECGData);
    }
}

bool ECG::isFlushFlagSet() const noexcept {
    return flushFlag == 1;
}

void ECG::resetFlushFlag() noexcept {
    flushFlag = 0;
}

uint16_t ECG::getFrontECGData() const noexcept {
    if (!ECGQueue.empty()) {
        return ECGQueue.front();
    }
    return 0;
}
uint16_t ECG::popFrontECGData() noexcept {
    if (!ECGQueue.empty()) {
        uint16_t front = ECGQueue.front();
        ECGQueue.pop();
        return front;
    }
    return 0;
}