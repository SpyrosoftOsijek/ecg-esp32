#include <iostream>
#include "esp_log.h"
#include "UARTManager.h"
#include "ECG.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" void app_main(void) {
    ESP_LOGI("Main", "Application startup");

    esp_log_set_level_master(ESP_LOG_WARN);

    ECG ecgData;
    UARTManager uartManager(ecgData);

    uartManager.init();
    
while (true) {
        uartManager.ECGDataGet();
        ecgData.displayData();
        vTaskDelay(10 / portTICK_PERIOD_MS); 
    }
}
 
    
