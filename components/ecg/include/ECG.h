#ifndef ECG_H
#define ECG_H

#include <cstdint>
#include "ServiceProvider.h"

class ECG {
private:
    ServiceProvider& serviceProvider;

public:
    ECG(ServiceProvider& serviceProviderRef) noexcept;
    void getData();
    void displayECGData();
};

#endif 
