#include "UARTManager.h"
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#define UART_TX_PIN 16
#define UART_RX_PIN 17

const char* EXAMPLE_TAG = "UARTManager";

UARTManager::UARTManager(uint8_t txPin, uint8_t rxPin)
    : soft_uart_port(nullptr), rxBuff{},
      flushFlag(false), ECGdataFSM(ECG_IDLE), ECGDataLength(0), dataCount(0), ECG16Bitdata(0) {
    initialize(txPin, rxPin); 
}

void UARTManager::parseECGData() {
    uint8_t data;
    size_t expected_read_size = 1024;

    esp_err_t ret = soft_uart_receive(soft_uart_port, rxBuff, expected_read_size);

    if (ret == ESP_OK) {
         ESP_LOGI(EXAMPLE_TAG, "Data received: %d bytes", expected_read_size);
        for (size_t i = 0; i < expected_read_size; i++) {
            data = rxBuff[i];
             ESP_LOGI(EXAMPLE_TAG, "Received byte: 0x%02x", data);
            switch (ECGdataFSM) {
                case ECG_IDLE:
                    if (data == 0xAA) {
                        ECGdataFSM = ECG_DATA_LENGTH;
                    }
                    break;
                case ECG_DATA_LENGTH:
                    ECGDataLength = data;
                    dataCount = 0;
                    ECGdataFSM = ECG_DATA_H;
                    break;
                case ECG_DATA_H:
                    ECG16Bitdata = data << 8;
                    ECGdataFSM = ECG_DATA_L;
                    break;
                case ECG_DATA_L:
                    ECG16Bitdata |= data;
                    ECGData16Buff[dataCount] = ECG16Bitdata;
                    dataCount++;
                    ECGdataFSM = (dataCount >= ECGDataLength) ? ECG_END : ECG_DATA_H;
                    break;
                case ECG_END:
                    if (data == 0xef) {
                        addECGData(ECGData16Buff, dataCount);
                        flushFlag = true;
                    }
                    ECGdataFSM = ECG_IDLE;
                    break;
            }
        }
    } else {
         ESP_LOGE(EXAMPLE_TAG, "Error receiving data: %s", esp_err_to_name(ret));
    }
}

void UARTManager::addECGData(const uint16_t* data, size_t length) { 
    for (size_t i = 0; i < length; i++) {
        dataQueue.push(data[i]);
    }
    flushFlag = true;
}



void UARTManager::initialize(uint8_t txPin, uint8_t rxPin) {
    esp_err_t ret = ESP_OK;

    soft_uart_config_t config = {
        .tx_pin = txPin,
        .rx_pin = rxPin,
        .baudrate = SOFT_UART_115200
    };

    ret = soft_uart_new(&config, &soft_uart_port);

    if (ret != ESP_OK) {
        ESP_LOGE(EXAMPLE_TAG, "Error initializing UART: %s", esp_err_to_name(ret));
    }
}
