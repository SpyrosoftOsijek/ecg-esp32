#ifndef UARTMANAGER_H
#define UARTMANAGER_H

#include "soft_uart.h"
#include "ECG.h"
#include <cstdint>

class UARTManager {
private:
    soft_uart_port_t soft_uart_port;
    ECG& ecgData;
   uint8_t rxBuff[1024];

    enum ECGState {
        ECG_IDLE,
        ECG_DATA_LENGTH,
        ECG_DATA_H,
        ECG_DATA_L,
        ECG_END,
    };
    uint8_t ECGdataFSM;
    uint8_t ECGDataLength;
    uint8_t dataCount;
    uint8_t flushFlag;
    uint16_t ECGData16Buff[128];
    uint16_t ECG16Bitdata;

public:

    UARTManager(ECG& ecgDataRef);
    void ECGDataGet();
    void init();
    uint8_t* getRxBuff();

};

#endif 
