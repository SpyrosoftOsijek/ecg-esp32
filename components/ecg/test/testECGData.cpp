#include "MockECGDataProvider.cpp"
#include "ECG.h"
#include "unity.h"
#include <sstream>
#include <iostream>
#include <queue>


TEST_CASE("First test for ECG receive", "[ECG]") {
    MockECGDataServiceProvider mockServiceProvider;
    ECG ecg(mockServiceProvider);

    mockServiceProvider.rxBuff[0] = 0xAA;
    mockServiceProvider.rxBuff[1] = 0x02;
    mockServiceProvider.rxBuff[2] = 0x12;
    mockServiceProvider.rxBuff[3] = 0x34;
    mockServiceProvider.rxBuff[4] = 0xef;

    ecg.getData();


    std::queue<uint16_t> testQueue = mockServiceProvider.getECGQueue();
    if (!testQueue.empty()) {
        TEST_ASSERT_EQUAL_HEX16(0x1234, testQueue.front());
    }
}

TEST_CASE("Second test for ECG receive", "[ECG]") {
    MockECGDataServiceProvider mockServiceProvider;
    ECG ecg(mockServiceProvider);

    mockServiceProvider.rxBuff[0] = 0xAA;
    mockServiceProvider.rxBuff[1] = 0x12;
    mockServiceProvider.rxBuff[2] = 0x00;
    mockServiceProvider.rxBuff[3] = 0x04;
    mockServiceProvider.rxBuff[4] = 0x03;
    mockServiceProvider.rxBuff[5] = 0x07;
    mockServiceProvider.rxBuff[6] = 0x11;
    mockServiceProvider.rxBuff[7] = 0x10;
    mockServiceProvider.rxBuff[8] = 0xef;


    ecg.getData();


    std::queue<uint16_t> testQueue = mockServiceProvider.getECGQueue();
    if (!testQueue.empty()) {
        testQueue.pop();
        TEST_ASSERT_EQUAL_HEX16(0x0403, testQueue.front());
    }
}