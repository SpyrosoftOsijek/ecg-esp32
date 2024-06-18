#ifndef UARTMANAGER_H
#define UARTMANAGER_H

#include "soft_uart.h"
#include "IECGDataProvider.h"
#include <cstdint>
#include <queue>


class UARTManager : public IECGDataProvider {
private:
    soft_uart_port_t soft_uart_port;
    uint8_t rxBuff[1024];
    std::queue<uint16_t> ECGQueue;
    bool flushFlag;

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
    uint16_t ECGData16Buff[128];
    uint16_t ECG16Bitdata;

public:
    UARTManager();
    void ECGDataGet() override;
    void addECGData(const uint16_t* data, size_t length) override;
    bool isFlushFlagSet() const override;
    void resetFlushFlag() override;
    void displayData() override;
    void init();
    uint8_t* getRxBuff();
    std::queue<uint16_t> getECGQueue() const ;

};

#endif 
