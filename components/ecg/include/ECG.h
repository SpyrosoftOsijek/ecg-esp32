#ifndef ECG_H
#define ECG_H

#include <queue>
#include <cstdint>

class ECG {
private:
    std::queue<uint16_t> ECGQueue;
    uint8_t flushFlag;

public:
    ECG() noexcept : flushFlag(false) {}

    void addECGData(const uint16_t* data, size_t length) noexcept;
    void displayData() noexcept;
    bool isFlushFlagSet() const noexcept;
    void resetFlushFlag() noexcept;
    uint16_t getFrontECGData() const noexcept;
    uint16_t popFrontECGData() noexcept;
};

#endif 
