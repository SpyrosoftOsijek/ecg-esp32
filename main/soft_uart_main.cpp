#include <iostream>
#include "esp_log.h"
#include "ECG.h"
#include "uartECGDataProvider.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

const char* MAIN_TAG = "Main";
static ECG* globalEcgInstance = nullptr;

extern "C" void app_main(void) {
    ESP_LOGI(MAIN_TAG, "Application startup");
    IECGDataProvider* uartDataProvider = new uartECGDataProvider();
    ECG* ecg = new ECG(*uartDataProvider);
    globalEcgInstance = ecg;

    ecg->startGatheringECGData();
    
}
