#include "MockECGDataProvider.cpp"
#include "ECG.h"
#include "unity.h"
#include <sstream>
#include <iostream>
#include <queue>


TEST_CASE("Test adding data to ECG queue", "[ECG]") {
    
    MockECGDataServiceProvider mockServiceProvider;
    ECG ecg(mockServiceProvider);

    uint16_t testData[] = {0x0001, 0x1111, 0x0203};
    size_t testDataLength = sizeof(testData) / sizeof(testData[0]);

    mockServiceProvider.addTestData(testData, testDataLength);


    std::queue<uint16_t>& dataQueue = mockServiceProvider.getECGQueue();

    if (!dataQueue.empty()) {
        TEST_ASSERT_EQUAL_UINT16(testData[0], dataQueue.front()); 
    }
}


TEST_CASE("Test length of test data vs added data to queue", "[ECG]") {
    
    MockECGDataServiceProvider mockServiceProvider;
    ECG ecg(mockServiceProvider);

    uint16_t testData[] = {0x0001, 0x1111, 0x0203, 0x0001, 0x0001, 0x0001};
    size_t testDataLength = sizeof(testData) / sizeof(testData[0]);

    mockServiceProvider.addTestData(testData, testDataLength);
    std::queue<uint16_t>& dataQueue = mockServiceProvider.getECGQueue();
    TEST_ASSERT_EQUAL(testDataLength, dataQueue.size());

}