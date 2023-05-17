#include "mpwm.h"

MPwmTimer::MPwmTimer(ledc_timer_bit_t dutyResolution, uint32_t freqHZ, ledc_timer_t timerNum, ledc_mode_t speedMode, ledc_clk_cfg_t clkCfg)
{
    esp_err_t err = ESP_OK;
    bTimerIntied_ = false;
    ledCtimer_.duty_resolution = dutyResolution;
    ledCtimer_.freq_hz = freqHZ;
    ledCtimer_.timer_num = timerNum;
    ledCtimer_.speed_mode = speedMode;
    ledCtimer_.clk_cfg = clkCfg;
    if((err = ledc_timer_config(&ledCtimer_)) != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
    }
    bTimerIntied_ = true;
}
MPwmTimer::~MPwmTimer()
{
    reset();
}
bool MPwmTimer::pause()
{
    esp_err_t err = ledc_timer_pause(ledCtimer_.speed_mode, ledCtimer_.timer_num);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}
bool MPwmTimer::resume()
{
    esp_err_t err = ledc_timer_resume(ledCtimer_.speed_mode, ledCtimer_.timer_num);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}
bool MPwmTimer::reset()
{
    esp_err_t err = ledc_timer_rst(ledCtimer_.speed_mode, ledCtimer_.timer_num);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}

uint32_t MPwmTimer::getFreq()
{
    return ledc_get_freq(ledCtimer_.speed_mode, ledCtimer_.timer_num);
}

bool MPwmTimer::setFreq(uint32_t freqHz)
{
    esp_err_t err = ledc_set_freq(ledCtimer_.speed_mode, ledCtimer_.timer_num, freqHz);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}
//-------------------------------------------------------------------------------------------------------------------------
MPwm::MPwm(int32_t gpioNum, MPwmTimer* ledcTimer):bfadeEnable_(false),gpioNum_(gpioNum),ledcTimer_(ledcTimer)
{

}
MPwm::~MPwm()
{
    if(bfadeEnable_)
    {
        disableFade();
    }
}

bool MPwm::channelConfig(uint32_t duty, ledc_channel_t channel, int32_t hpoint)
{
    esp_err_t err = ESP_OK;
    ledcChannel_.gpio_num = gpioNum_;
    ledcChannel_.speed_mode = ledcTimer_->getSpeedMode();
    ledcChannel_.channel = channel;
    ledcChannel_.intr_type = LEDC_INTR_DISABLE;
    ledcChannel_.timer_sel = ledcTimer_->getTimerNum();
    ledcChannel_.duty = duty;
    ledcChannel_.hpoint = hpoint;
    if((err = ledc_channel_config(&ledcChannel_)) != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}

bool MPwm::enableFade(int32_t intrAllocFlag)
{
    if(bfadeEnable_)
    {
        return true;
    }
    esp_err_t err = ledc_fade_func_install(intrAllocFlag);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    bfadeEnable_ = true;
    return true;
}
void MPwm::disableFade()
{
    if(bfadeEnable_)
    {
        ledc_fade_func_uninstall();
        bfadeEnable_ = false;
    }
}
bool MPwm::setFadeStepAndStart(uint32_t targetDuty, uint32_t scale, uint32_t cycleNum, ledc_fade_mode_t fadeMode)
{
    if(!bfadeEnable_)
    {
        printf("Error! setFadeStepAndStart need call enableFade() first\n");
        return false;
    }
    esp_err_t err = ledc_set_fade_step_and_start(ledcTimer_->getSpeedMode(),ledcChannel_.channel,targetDuty,scale,cycleNum,fadeMode);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}

bool MPwm::setFadeTimeAndStart(uint32_t targetDuty, uint32_t maXfadetimeMs, ledc_fade_mode_t fadeMode)
{
    if(!bfadeEnable_)
    {
        printf("Error! setFadeStepAndStart need call enableFade() first\n");
        return false;
    }
    esp_err_t err = ledc_set_fade_time_and_start(ledcTimer_->getSpeedMode(), ledcChannel_.channel, targetDuty, maXfadetimeMs, fadeMode);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}

bool MPwm::setDutyAndUpdate(uint32_t duty, uint32_t hpoint)
{
    esp_err_t err = ledc_set_duty_and_update(ledcTimer_->getSpeedMode(), ledcChannel_.channel, duty, hpoint);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}
bool MPwm::swSetDutyAndUpdate(uint32_t duty)
{
    esp_err_t err = ledc_set_duty(ledcChannel_.speed_mode, ledcChannel_.channel, duty);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    err = ledc_update_duty(ledcChannel_.speed_mode, ledcChannel_.channel);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}
uint32_t MPwm::getDuty()
{
    return ledc_get_duty(ledcTimer_->getSpeedMode(), ledcChannel_.channel);
}

bool MPwm::stop(uint32_t idleLevel)
{
    esp_err_t err = ledc_stop(ledcTimer_->getSpeedMode(), ledcChannel_.channel, idleLevel);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}
bool MPwm::fadeEndCbRegister(ledc_cb_t cb, void *useRarg)
{
    cbs_.fade_cb = cb;
    esp_err_t err = ledc_cb_register(ledcTimer_->getSpeedMode(), ledcChannel_.channel, &cbs_, useRarg);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}
