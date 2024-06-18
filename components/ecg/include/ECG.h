#ifndef ECG_H
#define ECG_H

#include <cstdint>
#include "IECGDataProvider.h"

class ECG {
private:
    IECGDataProvider& serviceProvider;

public:
    ECG(IECGDataProvider& serviceProviderRef) noexcept;
    void getData();
    void displayECGData();
};

#endif 
