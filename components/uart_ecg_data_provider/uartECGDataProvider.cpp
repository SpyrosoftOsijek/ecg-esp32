#include "uartECGDataProvider.h"
#include "esp_log.h"
#include "esp_check.h"

const char* UART_ECG_DATA_PROVIDER_TAG = "uartECGDataProvider";
const uint8_t START_BYTE = 0xAA;
const uint8_t END_BYTE = 0xef;
const size_t expected_read_size = 1024;

uartECGDataProvider::uartECGDataProvider()
    : soft_uart_port(nullptr), rxBuff{},
       ECGdataFSM(ECG_IDLE), ECGDataLength(0), dataCount(0), ECG16Bitdata(0) {
    initialize(CONFIG_EMULATE_UART_GPIO_TX, CONFIG_EMULATE_UART_GPIO_RX); 
}

bool uartECGDataProvider::isPacketValid(size_t startIdx) {
    size_t lengthIdx = startIdx + 1;
    size_t dataLength = rxBuff[lengthIdx];
    size_t endIdx = lengthIdx + 1 + (dataLength * 2);

    if (endIdx >= expected_read_size) {
        return false; 
    }
    if (rxBuff[endIdx] != END_BYTE) {
        return false; 
    }
    return true;
}

 esp_err_t uartECGDataProvider::pollECGData()  { 
    if (soft_uart_port == nullptr) {
        ESP_LOGE(UART_ECG_DATA_PROVIDER_TAG, "UART port not initialized");
        return ESP_FAIL;
    }

    esp_err_t ret = soft_uart_receive(soft_uart_port, rxBuff, expected_read_size);
    if (ret == ESP_OK) {
        parseECGData(ret);
    }
    return ret;
}

void uartECGDataProvider::parseECGData(esp_err_t ret) {
    if (ret == ESP_OK) {
        ESP_LOGI(UART_ECG_DATA_PROVIDER_TAG, "Data received: %d bytes", expected_read_size);
        size_t i = 0;
        bool startFound = false;

        while (i < expected_read_size) {
            uint8_t data = rxBuff[i];
            ESP_LOGI(UART_ECG_DATA_PROVIDER_TAG, "Received byte: 0x%02x at index %d", data, i);

            if (!startFound) {
                if (data == START_BYTE) {
                    startFound = true;
                    ECGDataLength = 0;
                    dataCount = 0;

                    if (!isPacketValid(i)) {
                        ESP_LOGE(UART_ECG_DATA_PROVIDER_TAG, "Data skipped because it did not contain end byte");
                        startFound = false;
                        i++;
                        continue;
                    }
                }
            } else if (ECGDataLength == 0) {
                ECGDataLength = data;
                dataCount = 0;
            } else if (dataCount < ECGDataLength) {
                uint16_t highByte = data;

                if (i + 1 < expected_read_size) {
                    uint16_t lowByte = rxBuff[++i];
                    ESP_LOGI(UART_ECG_DATA_PROVIDER_TAG, "Received byte: 0x%02x at index %d", lowByte, i);
                    ESP_LOGI(UART_ECG_DATA_PROVIDER_TAG, "High byte: 0x%02x at index %d", highByte, i - 1);
                    ESP_LOGI(UART_ECG_DATA_PROVIDER_TAG, "Low byte: 0x%02x at index %d", lowByte, i);
                    uint16_t ECG16Bitdata = (highByte << 8) | lowByte;
                    invokeCallback(ECG16Bitdata);
                    dataCount++;
                }
            } else {
                if (data == END_BYTE) {
                    ESP_LOGI(UART_ECG_DATA_PROVIDER_TAG, "End byte received");
                } else {
                    ESP_LOGW(UART_ECG_DATA_PROVIDER_TAG, "Warning: Last byte is not END_BYTE");
                }
                startFound = false;
                ECGDataLength = 0;
            }
            i++;
        }
    } else {
        ESP_LOGE(UART_ECG_DATA_PROVIDER_TAG, "Error receiving data: %s", esp_err_to_name(ret));
    }
}



void uartECGDataProvider::initialize(uint8_t txPin, uint8_t rxPin) {
    soft_uart_config_t config = {
        .tx_pin = txPin,
        .rx_pin = rxPin,
        .baudrate = SOFT_UART_115200
    };

    esp_err_t ret = soft_uart_new(&config, &soft_uart_port);
   if (ret != ESP_OK) {
       // ESP_GOTO_ON_FALSE(ret == ESP_OK, UART_ECG_DATA_PROVIDER_TAG, error, "Error initializing UART: %s", esp_err_to_name(ret)); // ne želim vidjet niakkav goto
   }
 
}
