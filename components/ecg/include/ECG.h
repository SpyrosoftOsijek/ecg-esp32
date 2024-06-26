#ifndef ECG_ECG_H
#define ECG_ECG_H

#include <cstdint>
#include "IECGDataProvider.h"

namespace ecgData{

    class ECG {
public:
    ECG(IECGDataProvider& ecgDataProviderRef) noexcept;
    const ECGDataQueue& getECGDataQueue() const;
    void displayECGData();
    void startGatheringECGData();
    
private:
    IECGDataProvider& ecgDataProvider;
    ECGDataQueue& dataQueue;
    static void pollTask(void* pvParameters);
};

}
#endif 

