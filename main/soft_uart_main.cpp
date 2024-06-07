/*
 * SPDX-FileCopyrightText: 2010-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#include <iostream>
#include <vector>
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_check.h"
#include "soft_uart.h"
#include "driver/uart.h"


#define READ_COUNT  16
#define UART_TX_PIN CONFIG_EMULATE_UART_GPIO_TX
#define UART_RX_PIN CONFIG_EMULATE_UART_GPIO_RX
#define BUF_SIZE 1024


enum ECGState {
     ECG_IDLE,
    ECG_DATA_LENGTH,
    ECG_DATA_H,
    ECG_DATA_L,
    ECG_END 
    };

uint8_t rxBuff[BUF_SIZE];
uint8_t ECGdataFSM = ECG_IDLE;
uint8_t ECGDataLength = 0, dataCount = 0, flushFlag = 0;
uint16_t ECGData16Buff[128];
uint16_t ECGDataBuff[640];
uint16_t ECG16Bitdata;
int16_t ECGWritePoint = 320;
int16_t ECGReadPoint = 0;

uint16_t countSerialData = 0;
unsigned long starttime = 0;
unsigned long DeltalTime = 0;
double dataDeltaTime = 0;


const char* EXAMPLE_TAG = "dedicated_gpio_example";


void ECGDataGet() {}

extern "C" void app_main(void)
{
    esp_err_t ret = ESP_OK;
    std::vector<uint8_t> read_buffer(READ_COUNT, 0);
    const uint8_t write_buffer[] = "Hello, world! This is a message.\r\n";
    uint8_t dummy = 0;
    soft_uart_port_t port = NULL;
    soft_uart_config_t config = {
        .tx_pin = CONFIG_EMULATE_UART_GPIO_TX,
        .rx_pin = CONFIG_EMULATE_UART_GPIO_RX,
        .baudrate = SOFT_UART_115200
    };

 

    /* Initialize and configure the software UART port */
    ret = soft_uart_new(&config, &port);
    ESP_GOTO_ON_ERROR(ret, error, EXAMPLE_TAG, "Error configuring software UART");

while(true){


  
    
 ECGDataGet();

 
  ret = soft_uart_receive(port, read_buffer.data(), read_buffer.size());
    ESP_GOTO_ON_ERROR(ret, error, EXAMPLE_TAG, "Error reading from the software UART");

    std::cout << "UART transfers succeeded, received bytes: { ";
    for (auto byte : read_buffer) {
        std::cout << "0x" << std::hex << static_cast<int>(byte) << " ";
    }
    std::cout << "}" << std::endl;
}
    

error:
    if (port != NULL) {
        soft_uart_del(port);
    }
    if (ret != ESP_OK) {
        ESP_LOGE(EXAMPLE_TAG, "An error occurred while communicating through the UART");
    }
}

/*
void ECGDataGet()
{
    uint8_t data;


   uint8_t data;
    int length = uart_read_bytes(UART_NUM, rxBuff, BUF_SIZE, 100 / portTICK_RATE_MS);

    if (length > 0)
    {


         for (int i = 0; i < length; i++)
        {
            data = rxBuff[i];


            
            switch (ECGdataFSM) {
                case ECG_IDLE:
                    ECGdataFSM = (data == 0xAA) ? ECG_DATA_LENGTH : ECG_IDLE;
                    break;
                case ECG_DATA_LENGTH:
                    ECGDataLength = data;
                    dataCount = 0;
                    ECGdataFSM = ECG_DATA_H;
                    break;
                case ECG_DATA_H:
                    ECG16Bitdata = 0;
                    ECG16Bitdata = data;
                    ECG16Bitdata <<= 8;
                    ECGdataFSM = ECG_DATA_L;
                    break;
                case ECG_DATA_L:
                    ECG16Bitdata |= data;
                    ECGData16Buff[dataCount] = ECG16Bitdata;
                    dataCount++;
                    ECGdataFSM = (dataCount >= ECGDataLength) ? ECG_END : ECG_DATA_H;
                    break;
                case ECG_END:
                    if (data == 0xEF) {
                        ECGDataBuff[(ECGWritePoint % 640)] = ECGData16Buff[0];
                        ECGWritePoint = (ECGWritePoint + 1) % 640;
                        ECGReadPoint = (ECGReadPoint + 1) % 640;
                        flushFlag = true;
                    }
                    ECGdataFSM = ECG_IDLE;
                    break;
            }
        }
    }
}
*/
