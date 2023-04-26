#include "madc.h"
bool MAdc::bOneShortUnitInit_ = false;
adc_oneshot_unit_handle_t MAdc::handle_ = nullptr;
adc_oneshot_unit_init_cfg_t MAdc::unitCfg_;

MAdc::MAdc() : bCalibrated_(false)
{

}
MAdc::~MAdc()
{
    caliFittingDestory();
    if(bOneShortUnitInit_)
    {
        oneShortUnitDeInit();
    }
}
bool MAdc::oneShortUnitInit(adc_unit_t unitID, adc_ulp_mode_t mode)
{
    if(bOneShortUnitInit_)
    {
        if(!oneShortUnitDeInit())
        {
            return false;
        }
    }
    unitCfg_.unit_id = unitID;
    unitCfg_.ulp_mode = mode;

    esp_err_t err = adc_oneshot_new_unit(&unitCfg_, &handle_);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    bOneShortUnitInit_ = true;
    return true;
}
bool MAdc::oneShortUnitDeInit()
{
    if(!bOneShortUnitInit_)
    {
        return false;
    }
    bOneShortUnitInit_ = false;
    esp_err_t err = adc_oneshot_del_unit(handle_);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}

bool MAdc::oneShortChanConfig(adc_channel_t channel, adc_atten_t atten, adc_bitwidth_t bitwidth)
{
    return oneShortChanConfig_(channel, atten, bitwidth);
}
bool MAdc::oneShortChanConfig(gpio_num_t gpio, adc_atten_t atten, adc_bitwidth_t bitwidth)
{
    esp_err_t err = adc_oneshot_io_to_channel(static_cast<int>(gpio), &unitCfg_.unit_id, &channel_);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return oneShortChanConfig_(channel_, atten, bitwidth);
}
bool MAdc::oneShortChanConfig_(adc_channel_t channel, adc_atten_t atten, adc_bitwidth_t bitwidth)
{
    if(!bOneShortUnitInit_)
    {
        printf("Error: when config chan, please init adc unit first\n");
        return false;
    }
    channel_ = channel;
    chanCfg_.atten = atten;
    chanCfg_.bitwidth = bitwidth;
    esp_err_t err = adc_oneshot_config_channel(handle_, channel, &chanCfg_);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}
bool MAdc::oneShortRead(int *out_raw)
{
    if(!bOneShortUnitInit_)
    {
        printf("Error: when config chan, please init adc unit first\n");
        return false;
    }
    esp_err_t err = adc_oneshot_read(handle_, channel_, out_raw);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}
bool MAdc::getGpioNum(int* gpio)
{
    esp_err_t err = adc_oneshot_channel_to_io(unitCfg_.unit_id, channel_, gpio);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}
bool MAdc::caliFittingConfig(uint32_t defaultVref)
{
    if(bCalibrated_)
    {
        printf("Warning: %s()%d %s\n",__FUNCTION__,__LINE__,"cali already configed!");
        return true;
    }
    esp_err_t err = ESP_FAIL;
    adc_cali_scheme_ver_t schemeMask_;
    adc_cali_check_scheme(&schemeMask_);
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    caliCurveFittingCfg_.unit_id = unitCfg_.unit_id;
    caliCurveFittingCfg_.atten = chanCfg_.atten;
    caliCurveFittingCfg_.bitwidth = chanCfg_.bitwidth;
    if(schemeMask_ == ADC_CALI_SCHEME_VER_CURVE_FITTING)
    {
        err = adc_cali_create_scheme_curve_fitting(&caliCurveFittingCfg_, &caliHandle_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        bCalibrated_ = true;
    }

#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    caliLineFittingCfg_.unit_id = unitCfg_.unit_id;
    caliLineFittingCfg_.atten = chanCfg_.atten;
    caliLineFittingCfg_.bitwidth = chanCfg_.bitwidth;

#if CONFIG_IDF_TARGET_ESP32
    adc_cali_line_fitting_efuse_val_t cali_val;
    err = adc_cali_scheme_line_fitting_check_efuse(&cali_val);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
    }
    if(cali_val == ADC_CALI_LINE_FITTING_EFUSE_VAL_EFUSE_VREF)
    {
        caliLineFittingCfg_.default_vref = defaultVref;
    }
#endif
    if(schemeMask_ == ADC_CALI_SCHEME_VER_LINE_FITTING)
    {
        err = adc_cali_create_scheme_line_fitting(&caliLineFittingCfg_, &caliHandle_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        bCalibrated_ = true;
    }
#endif
    return true;
}
bool MAdc::caliFittingDestory()
{
    if(!bCalibrated_ || caliHandle_ == nullptr)
    {
        return true;
    }
    esp_err_t err = ESP_FAIL;
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    err = adc_cali_delete_scheme_curve_fitting(caliHandle_);
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    err = adc_cali_delete_scheme_line_fitting(caliHandle_);
#endif
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}
bool MAdc::caliRawToVoltage(int *voltage)
{
    if(!bCalibrated_)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,"Can not get Voltage");
        return false;
    }
    int raw = 0;
    if(!oneShortRead(&raw))
    {
        return false;
    }
    esp_err_t err = adc_cali_raw_to_voltage(caliHandle_, raw, voltage);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}