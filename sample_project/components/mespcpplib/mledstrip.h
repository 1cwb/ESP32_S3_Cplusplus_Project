#pragma once
#include "esp_check.h"
#include "mrmt.h"

#define RMT_LED_STRIP_RESOLUTION_HZ 10000000 // 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define RMT_LED_STRIP_GPIO_NUM      GPIO_NUM_48

#define EXAMPLE_LED_NUMBERS         3
#define EXAMPLE_CHASE_SPEED_MS      10

class LedStripEncoder : public RmtEncoderBase
{
public:
    bool init(uint32_t resolution);

    rmt_symbol_word_t* getResetCode() {return &resetCode_;}
private:
    void setResetCode(uint32_t resetTicks)
    {
        resetCode_.level0 = 0;
        resetCode_.duration0 = resetTicks;
        resetCode_.level1 = 0;
        resetCode_.duration1 = resetTicks;
    }
    rmt_symbol_word_t resetCode_;
};

class LedStrip
{
public:
    bool init();
    bool setRGB(uint32_t red, uint32_t green, uint32_t blue);
    bool update();
    bool setRGBAndUpdate(uint32_t red, uint32_t green, uint32_t blue);
private:
    MRmtTx mrttx_;
    LedStripEncoder ledEncoder_;
    rmt_transmit_config_t txConfig_;
    uint8_t ledStripPixels_[EXAMPLE_LED_NUMBERS];
};