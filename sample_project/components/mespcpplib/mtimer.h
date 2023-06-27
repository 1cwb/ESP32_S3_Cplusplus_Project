#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gptimer.h"

class MTimer
{
public:
    //using timerIntrCb = void (*)(void *);
    MTimer();
    ~MTimer();
    MTimer(const MTimer&) = delete;
    MTimer(MTimer&&) = delete;
    MTimer& operator=(const MTimer& ) = delete;
    MTimer& operator=(MTimer&& ) = delete;
 
    bool init(uint32_t resolutionHz, gptimer_count_direction_t direction = GPTIMER_COUNT_UP, gptimer_clock_source_t clkSrc = GPTIMER_CLK_SRC_DEFAULT, bool bIntrShared = false);
    bool deinit();
    bool enable();
    bool disable();
    bool start();
    bool stop();
    bool setRawCount(const uint64_t value);
    bool getRawCount(uint64_t* value);
    uint32_t getTotalGpTimerNumers() const {return SOC_TIMER_GROUP_TOTAL_TIMERS;}
    uint32_t getGpTimerCountBitWidth() const {return  SOC_TIMER_GROUP_COUNTER_BIT_WIDTH;}

    bool enableAlarm(uint64_t alarmCount, uint64_t reloadCount, bool autoRealoadOnAlarm);
    bool updateAlarmValue(uint64_t alarmCount);
    bool disableAlarm();
    bool registerEventCallbacks(const gptimer_alarm_cb_t cbs, void *userData);
    bool started() const
    {
        return started_;
    }
private:
    bool started_;
    gptimer_handle_t handle_;
    gptimer_config_t config_;
    gptimer_alarm_config_t alarmConfig_;
    gptimer_event_callbacks_t cbs_;
};