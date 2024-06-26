#include <iostream>
#include <memory>
#include "esp_log.h"
#include "ECG.h"
#include "uartECGDataProvider.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

const char* MAIN_TAG = "Main";
//static ecgData::ECG* globalEcgInstance = nullptr;

extern "C" void app_main(void) {
    ESP_LOGI(MAIN_TAG, "Application startup");

    auto uartDataProvider = std::make_unique<ecgData::uartECGDataProvider>();
    auto ecg = std::make_unique<ecgData::ECG>(*uartDataProvider);

    ecg->startGatheringECGData();
}
