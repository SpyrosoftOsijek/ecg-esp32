#include <iostream>
#include "esp_log.h"
#include "ECG.h"
#include "UARTManager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" void app_main(void) {
    ESP_LOGI("Main", "Application startup");

    //esp_log_set_level_master(ESP_LOG_WARN);

    UARTManager uartManager(16,17);


    ECG ecg(uartManager);

    while (true) {
        uartManager.parseECGData();
        ecg.displayECGData();
        vTaskDelay(100 / portTICK_PERIOD_MS); 
    }
}
