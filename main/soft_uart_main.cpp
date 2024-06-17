#include <iostream>
#include "esp_log.h"
#include "ECG.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" void app_main(void) {
    ESP_LOGI("Main", "Application startup");

    esp_log_set_level_master(ESP_LOG_WARN);

    ECG ecgData;

    ecgData.start();
}
