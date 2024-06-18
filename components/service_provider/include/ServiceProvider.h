#ifndef SERVICE_PROVIDER_H
#define SERVICE_PROVIDER_H

#include <cstddef>
#include <cstdint>

class ServiceProvider {
public:
    virtual void ECGDataGet() = 0;
    virtual void addECGData(const uint16_t* data, size_t length) = 0;
    virtual bool isFlushFlagSet() const = 0;
    virtual void resetFlushFlag() = 0;
    virtual void displayData() = 0;
    virtual ~ServiceProvider() = default;
};

#endif
