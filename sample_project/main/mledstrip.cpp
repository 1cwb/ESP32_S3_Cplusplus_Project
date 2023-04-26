#include "mledstrip.h"
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static size_t rmt_encode_led_strip(rmt_encoder_t *encoder, rmt_channel_handle_t channel, const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state);
static esp_err_t rmt_del_led_strip_encoder(rmt_encoder_t *encoder);
static esp_err_t rmt_led_strip_encoder_reset(rmt_encoder_t *encoder);

bool LedStripEncoder::init(uint32_t resolution)
{
    setBaseEncoderCb(rmt_encode_led_strip);
    setBaseResetCb(rmt_led_strip_encoder_reset);
    setBaseDelCb(rmt_del_led_strip_encoder);
    setResetCode(resolution / 1000000 * 50 / 2);

    rmt_bytes_encoder_config_t bytes_encoder_config;
    bytes_encoder_config.bit0.level0 = 1;
    bytes_encoder_config.bit0.duration0 = 0.3 * resolution / 1000000, // T0H=0.3us
    bytes_encoder_config.bit0.level1 = 0;
    bytes_encoder_config.bit0.duration1 = 0.9 * resolution / 1000000, // T0L=0.9us

    bytes_encoder_config.bit1.level0 = 1;
    bytes_encoder_config.bit1.duration0 = 0.9 * resolution / 1000000, // T1H=0.9us
    bytes_encoder_config.bit1.level1 = 0;
    bytes_encoder_config.bit1.duration1 = 0.3 * resolution / 1000000, // T1L=0.3us
    bytes_encoder_config.flags.msb_first = 1;

    rmt_copy_encoder_config_t copy_encoder_config = {};

    if(!initBytesEcoder(&bytes_encoder_config))
    {
        return false;
    }
    if(!initCopyEcoder(&copy_encoder_config))
    {
        return false;
    }
    return true;
}
////////////////////////////////////LedStrip//////////////////////////////////////////////////////////

bool LedStrip::init()
{
    if(!mrttx_.init(RMT_LED_STRIP_GPIO_NUM, RMT_LED_STRIP_RESOLUTION_HZ, 64, 4 ,false, false, false, false))
    {
        return false;
    }
    if(!ledEncoder_.init(RMT_LED_STRIP_RESOLUTION_HZ))
    {
        return false;
    }
    mrttx_.enable();
    txConfig_.loop_count = 0;
    return true;
}
bool LedStrip::setRGB(uint32_t red, uint32_t green, uint32_t blue)
{
    memset(ledStripPixels_, 0, sizeof(ledStripPixels_));
    int i = 0;
    ledStripPixels_[i++] = 0xff & green;
    ledStripPixels_[i++] = 0xff & red;
    ledStripPixels_[i] = 0xff & blue;

    return true;
}
bool LedStrip::update()
{
    return mrttx_.transmit(ledEncoder_.getEncoderHandle(), ledStripPixels_, sizeof(ledStripPixels_), &txConfig_);
}
bool LedStrip::setRGBAndUpdate(uint32_t red, uint32_t green, uint32_t blue)
{
    setRGB(red, green, blue);
    return update();
}
size_t rmt_encode_led_strip(rmt_encoder_t *encoder, rmt_channel_handle_t channel, const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state)
{
    LedStripEncoder* baseEncoder =  reinterpret_cast<LedStripEncoder*>(encoder);
    rmt_encoder_handle_t bytesEncoder = baseEncoder->getBytesEncoder();
    rmt_encoder_handle_t copyEncoder = baseEncoder->getCopyEncoder();
    rmt_encode_state_t sessionState = RMT_ENCODING_MEM_FULL;
    rmt_encode_state_t state = RMT_ENCODING_MEM_FULL;
    size_t encodedSymbols = 0;
    do{
        if(baseEncoder->getState() == 0)
        {
            encodedSymbols += bytesEncoder->encode(bytesEncoder, channel, primary_data, data_size, &sessionState);
            if(sessionState & RMT_ENCODING_COMPLETE)
            {
                baseEncoder->setState(1);
            }
            if(sessionState & RMT_ENCODING_MEM_FULL)
            {
                state = (rmt_encode_state_t)(RMT_ENCODING_MEM_FULL);
                break;
            }
        }
        if(baseEncoder->getState() == 1)
        {
            encodedSymbols += copyEncoder->encode(copyEncoder, channel, baseEncoder->getResetCode(), sizeof(rmt_symbol_word_t), &sessionState);
            if(sessionState & RMT_ENCODING_COMPLETE)
            {
                baseEncoder->setState(0);
                state = (rmt_encode_state_t)RMT_ENCODING_COMPLETE; 
            }
            if(sessionState & RMT_ENCODING_MEM_FULL)
            {
                state = (rmt_encode_state_t)RMT_ENCODING_MEM_FULL;
                break;
            }
        }
    }while(false);
 
    *ret_state = state;
    return encodedSymbols;
}

esp_err_t rmt_del_led_strip_encoder(rmt_encoder_t *encoder)
{
    LedStripEncoder* baseEncoder =  reinterpret_cast<LedStripEncoder*>(encoder);
    baseEncoder->deInitBytesEcoder();
    baseEncoder->deInitCopyEcoder();
    return ESP_OK;
}

esp_err_t rmt_led_strip_encoder_reset(rmt_encoder_t *encoder)
{
    LedStripEncoder* baseEncoder =  reinterpret_cast<LedStripEncoder*>(encoder);
    baseEncoder->resetBytesEcoder();
    baseEncoder->resetCopyEcoder();
    baseEncoder->setState(0);
    return ESP_OK;
}