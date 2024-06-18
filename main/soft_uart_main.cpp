#include <iostream>
#include "esp_log.h"
#include "ECG.h"
#include "UARTManager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" void app_main(void) {
    ESP_LOGI("Main", "Application startup");

    esp_log_set_level_master(ESP_LOG_WARN);

    UARTManager uartManager;
    uartManager.init();

    ECG ecg(uartManager);

    while (true) {
        ecg.getData();
        ecg.displayECGData();
        vTaskDelay(10 / portTICK_PERIOD_MS); 
    }
}
