#pragma once
#include "mlcd.h"
#include "muiinterface.h"

class LcdDriver : public MUiInterface
{
public:
    LcdDriver():lcd_(new MLcd(35, 0, 320, 170, 170, 320))
    {
        lcd_->setBackLight(100);
        lcd_->setAddress( 0, 0,  lcd_->getWidth() - 1, lcd_->getHeight() - 1);
    }
    virtual ~LcdDriver()
    {
        if(lcd_)
        {
            delete lcd_;
        }
    }
    virtual uint16_t getWidth() const
    {
        return lcd_->getWidth();
    }
    virtual uint16_t getHeight() const
    {
        return lcd_->getHeight();
    }
    virtual uint32_t getMaxDataSendSize() const
    {
        return lcd_->getMaxDataSendSize();
    }
    virtual uint8_t getPixelBytes() const
    {
        return lcd_->getPixelBytes();
    }
    virtual void setAddress( uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
    {
        lcd_->setAddress(x1, y1, x2, y2);
    }
    virtual void sendData(const uint8_t* data, size_t len)
    {
        lcd_->lcdData(data, len);
    }
    virtual void setBackLight(uint8_t percent)
    {
        lcd_->setBackLight(percent);
    }
private:
    MLcd* lcd_;//(35, 0, 320, 170, 170, 320);
};