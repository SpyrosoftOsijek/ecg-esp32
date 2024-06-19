#ifndef ECG_H
#define ECG_H

#include <cstdint>
#include "IECGDataProvider.h"
#include <queue>

class ECG {

public:
    ECG(IECGDataProvider& serviceProviderRef) noexcept;
    const ECGDataQueue& getECGDataQueue() const;
    void displayECGData();
    
private:
    IECGDataProvider& serviceProvider;
    ECGDataQueue& dataQueue;
};

#endif 
