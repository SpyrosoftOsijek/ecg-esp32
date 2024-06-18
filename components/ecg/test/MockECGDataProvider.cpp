#include "IECGDataProvider.h"
#include "unity.h"
#include <queue>
#include <sstream>
#include <iostream>


class MockECGDataServiceProvider : public IECGDataProvider {
public:
    MockECGDataServiceProvider() : flushFlag(false), ECGdataFSM(ECG_IDLE), ECGDataLength(0), dataCount(0), ECG16Bitdata(0) {}

    uint8_t rxBuff[1024];


    void ECGDataGet() override {
        uint8_t data;
        size_t expected_read_size = sizeof(rxBuff);

        for (size_t i = 0; i < expected_read_size; i++) {
            data = rxBuff[i];
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
    }

    void displayData() override {
        while (!ECGQueue.empty()) {
            uint16_t ECGData = ECGQueue.front();
            ECGQueue.pop();
            printf("Mock ECG Data: 0x%04x\n", ECGData);
        }
    }

    bool isFlushFlagSet() const override {
        return flushFlag;
    }

    void resetFlushFlag() override {
        flushFlag = false;
    }

    std::queue<uint16_t> getECGQueue() const {
        return ECGQueue;
    }

private:
    void addECGData(const uint16_t* data, size_t length) {
        for (size_t i = 0; i < length; i++) {
            ECGQueue.push(data[i]);
        }
    }

    std::queue<uint16_t> ECGQueue;
    bool flushFlag;

    enum ECGState {
        ECG_IDLE,
        ECG_DATA_LENGTH,
        ECG_DATA_H,
        ECG_DATA_L,
        ECG_END
    } ECGdataFSM;

    uint16_t ECGData16Buff[256];
    uint16_t ECGDataLength;
    uint16_t dataCount;
    uint16_t ECG16Bitdata;
};
