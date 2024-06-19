#ifndef UARTMANAGER_H
#define UARTMANAGER_H

#include "soft_uart.h"
#include "IECGDataProvider.h"
#include <cstdint>
#include <queue>


class UARTManager : public IECGDataProvider {


    enum ECGState {
        ECG_IDLE,
        ECG_DATA_LENGTH,
        ECG_DATA_H,
        ECG_DATA_L,
        ECG_END,
    };
    

public:
    UARTManager(uint8_t txPin, uint8_t rxPin); 
    void parseECGData() override; 

private:
    void addECGData(const uint16_t* data, size_t length) override;
    void initialize(uint8_t txPin, uint8_t rxPin); 

    soft_uart_port_t soft_uart_port;
    uint8_t rxBuff[1024];

    uint8_t ECGdataFSM;
    uint8_t ECGDataLength;
    uint8_t dataCount;
    uint16_t ECGData16Buff[128];
    uint16_t ECG16Bitdata;
};

#endif 
