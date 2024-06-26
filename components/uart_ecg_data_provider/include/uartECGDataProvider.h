#ifndef UART_ECG_DATA_PROVIDER_UARTECGDATAPROVIDER_H
#define UART_ECG_DATA_PROVIDER_UARTECGDATAPROVIDER_H

#include "soft_uart.h"
#include "IECGDataProvider.h"
#include <cstdint>
#include <queue>

#define UART_TX_PIN CONFIG_EMULATE_UART_GPIO_TX
#define UART_RX_PIN CONFIG_EMULATE_UART_GPIO_RX

namespace ecgData{

class uartECGDataProvider : public IECGDataProvider {
    enum ECGState {
        ECG_IDLE,
        ECG_DATA_LENGTH,
        ECG_DATA_H,
        ECG_DATA_L,
        ECG_END,
    };
    
public:
    uartECGDataProvider(); 
    bool pollECGData() override;

private:
    void initialize(uint8_t txPin, uint8_t rxPin); 
    bool isPacketValid(size_t);
    void parseECGData(esp_err_t);
    
    soft_uart_port_t soft_uart_port;
    uint8_t rxBuff[1024];
    uint8_t ECGdataFSM;
    uint8_t ECGDataLength;
    uint8_t dataCount;
    uint16_t ECG16Bitdata;

};
}
#endif 
