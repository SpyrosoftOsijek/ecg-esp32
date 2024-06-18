#include "ECG.h"
#include "esp_log.h"

ECG::ECG(ServiceProvider& serviceProviderRef) noexcept
    : serviceProvider(serviceProviderRef) {}

void ECG::getData() {
    serviceProvider.ECGDataGet();
}

void ECG::displayECGData() {
    serviceProvider.displayData();
}
