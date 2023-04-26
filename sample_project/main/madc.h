#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

class MAdc
{
public:
    MAdc();
    ~MAdc();
    MAdc(const MAdc&) = delete;
    MAdc(MAdc&&) = delete;
    MAdc& operator=(const MAdc&) =delete;
    MAdc& operator=(MAdc&&) =delete;
    static bool oneShortUnitInit(adc_unit_t unitID, adc_ulp_mode_t mode);
    static bool oneShortUnitDeInit();
    bool oneShortChanConfig(adc_channel_t channel, adc_atten_t atten, adc_bitwidth_t bitwidth);
    bool oneShortChanConfig(gpio_num_t gpio, adc_atten_t atten, adc_bitwidth_t bitwidth);
    bool oneShortRead(int *out_raw);
    bool getGpioNum(int* gpio);
    adc_channel_t getChannel() const {return channel_;}

    bool caliFittingConfig(uint32_t defaultVref = 1100);
    bool caliFittingDestory();
    bool calibrated() const {return bCalibrated_;}
    bool caliRawToVoltage(int *voltage);

private:
    bool oneShortChanConfig_(adc_channel_t channel, adc_atten_t atten, adc_bitwidth_t bitwidth);

    adc_channel_t channel_;
    adc_oneshot_chan_cfg_t chanCfg_;
    adc_cali_handle_t caliHandle_;
    static bool bOneShortUnitInit_;
    static adc_oneshot_unit_handle_t handle_;
    static adc_oneshot_unit_init_cfg_t unitCfg_;

    bool bCalibrated_;
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_curve_fitting_config_t caliCurveFittingCfg_;

#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    adc_cali_line_fitting_config_t caliLineFittingCfg_;
#endif
};