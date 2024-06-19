#include "IECGDataProvider.h"
#include "unity.h"
#include <queue>
#include <sstream>
#include <iostream>


class MockECGDataServiceProvider : public IECGDataProvider {
public:
    MockECGDataServiceProvider() {}


 void parseECGData() override {
    }


  
    void addTestData(const uint16_t* data, size_t length) {
        addECGData(data, length);
    }


    

private:
    void addECGData(const uint16_t* data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        dataQueue.push(data[i]);
    }
}

   
};
