#pragma once
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"

class MPwmTimer
{
public:
    MPwmTimer(ledc_timer_bit_t dutyResolution = LEDC_TIMER_13_BIT, uint32_t freqHZ = 5000, ledc_timer_t timerNum = LEDC_TIMER_0, ledc_mode_t speedMode = LEDC_LOW_SPEED_MODE, ledc_clk_cfg_t clkCfg = LEDC_AUTO_CLK);
    ~MPwmTimer();
    MPwmTimer(const MPwmTimer&) = delete;
    MPwmTimer(MPwmTimer&&) = delete;
    MPwmTimer& operator=(const MPwmTimer&) =delete;
    MPwmTimer& operator=(MPwmTimer&&) =delete;

    bool timerInited() const {return bTimerIntied_;}
    ledc_timer_t getTimerNum() const {return ledCtimer_.timer_num;}
    ledc_mode_t getSpeedMode() const {return ledCtimer_.speed_mode;}
    bool pause();
    bool resume();
    bool reset();
    uint32_t getFreq();
    bool setFreq(uint32_t freqHz);
private:
    bool bTimerIntied_;
    ledc_timer_config_t ledCtimer_;
};

class MPwm
{
public:
    MPwm(int32_t gpioNum, MPwmTimer* lcdTimer);
    ~MPwm();
    MPwm(const MPwm&) = delete;
    MPwm(MPwm&&) = delete;
    MPwm& operator=(const MPwm&) =delete;
    MPwm& operator=(MPwm&&) =delete;
    bool channelConfig(uint32_t duty = 0, ledc_channel_t channel = LEDC_CHANNEL_0, int32_t hpoint = 0);
    bool enableFade(int32_t intrAllocFlag = 0);
    void disableFade();
    bool FadeEnabled() const {return bfadeEnable_;}
    //scale ： 每次增加duty的步长
    //cycleNum ： 多少个周期增加一次步长
    //example: 5000hz freq, duty = 8000, scale = 10, cycleNum = 10, 
    //         1ms freq = 5000/1000 = 5hz, 每过2ms 占空比+10， timer = 8000/10*2 = 800*2 = 1600ms;
    bool setFadeStepAndStart(uint32_t targetDuty = 0, uint32_t scale = 10, uint32_t cycleNum = 10, ledc_fade_mode_t fadeMode = LEDC_FADE_NO_WAIT);
    bool setFadeTimeAndStart(uint32_t targetDuty = 0, uint32_t maXfadetimeMs = 1000, ledc_fade_mode_t fadeMode = LEDC_FADE_NO_WAIT);

    bool setDutyAndUpdate(uint32_t duty, uint32_t hpoint = 0);
    bool swSetDutyAndUpdate(uint32_t duty);
    uint32_t getDuty();
    bool stop(uint32_t idleLevel = 0);
    bool fadeEndCbRegister(ledc_cb_t cb, void *useRarg);
private:
    bool bfadeEnable_;
    int32_t gpioNum_;
    MPwmTimer* ledcTimer_;
    ledc_channel_config_t ledcChannel_;
    ledc_cbs_t cbs_;
};