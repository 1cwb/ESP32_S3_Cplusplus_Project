#pragma once

class MUiInterface
{
public:
    MUiInterface() = default;
    virtual ~MUiInterface() = default;
    virtual uint16_t getWidth() const = 0;
    virtual uint16_t getHeight() const = 0;
    virtual uint32_t getMaxDataSendSize() const = 0;
    virtual uint8_t getPixelBytes() const = 0;
    virtual void setAddress( uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) = 0;
    virtual void sendData(const uint8_t* data, size_t len) = 0;
private:
};