#include "UARTManager.h"
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#define UART_TX_PIN 16
#define UART_RX_PIN 17

const char* EXAMPLE_TAG = "UARTManager";

UARTManager::UARTManager()
    : soft_uart_port(nullptr), rxBuff{},
      flushFlag(false), ECGdataFSM(ECG_IDLE), ECGDataLength(0), dataCount(0), ECG16Bitdata(0) {}

void UARTManager::ECGDataGet() {
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
        ECGQueue.push(data[i]);
    }
    flushFlag = true;
}

bool UARTManager::isFlushFlagSet() const {
    return flushFlag;
}

void UARTManager::resetFlushFlag() {
    flushFlag = false;
}

void UARTManager::init() {
    esp_err_t ret = ESP_OK;

    soft_uart_config_t config = {
        .tx_pin = UART_TX_PIN,
        .rx_pin = UART_RX_PIN,
        .baudrate = SOFT_UART_115200
    };

    ret = soft_uart_new(&config, &soft_uart_port);

}

uint8_t* UARTManager::getRxBuff() {
    return rxBuff;
}

void UARTManager::displayData() {
    std::queue<uint16_t> tempQueue = ECGQueue;

    if (!flushFlag) {
        return;
    }
    flushFlag = false;

    while (!tempQueue.empty()) {
        uint16_t ECGData = tempQueue.front();
        tempQueue.pop();
        ESP_LOGI(EXAMPLE_TAG, "ECG Data: 0x%04x", ECGData);
    }
}
