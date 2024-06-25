#include "IECGDataProvider.h"
#include "unity.h"
#include <queue>

class MockECGDataServiceProvider : public IECGDataProvider {
public:
    MockECGDataServiceProvider() {}
    
     esp_err_t pollECGData() {
        return ESP_OK ;
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
