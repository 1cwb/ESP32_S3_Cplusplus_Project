#pragma once 
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "sdkconfig.h"
#include <functional>

class MEspTimer
{
    using EvnetOnTimerCallback = std::function<void()>; 
public:
    MEspTimer(const char* name, esp_timer_dispatch_t dispatchMethod = ESP_TIMER_TASK, bool skipUnhandledEvents = true)
    :bstartOnce_(true), bstarted_(false), timerHandle_(nullptr), timerArgs_(nullptr), onTimerCb_(nullptr)
    {
        timerArgs_ = new esp_timer_create_args_t;
        if(!timerArgs_)
        {
            printf("Error: %s()%d new esp_timer_create_args_t fail\n",__FUNCTION__,__LINE__);
        }
        onTimerCb_ = new EvnetOnTimerCallback;
        if(!onTimerCb_)
        {
            printf("Error: %s()%d new EvnetOnTimerCallback fail\n",__FUNCTION__,__LINE__);
        }
        if(!init(name, dispatchMethod, skipUnhandledEvents))
        {
            printf("Error: %s()%d Init fail\n",__FUNCTION__,__LINE__);
        }
    }
    ~MEspTimer()
    {
        if(timerArgs_)
        {
            delete timerArgs_;
            timerArgs_ = nullptr;
        }
        if(onTimerCb_)
        {
            delete onTimerCb_;
            onTimerCb_ = nullptr;
        }
        if(bstarted_)
        {
            stop();
        }
        deinit();
    }
    MEspTimer(const MEspTimer&) = delete;
    MEspTimer(MEspTimer&&) = delete;
    MEspTimer& operator=(const MEspTimer& ) = delete;
    MEspTimer& operator=(MEspTimer&& ) = delete;

    void registerOnTimerCallback(const EvnetOnTimerCallback& cb)
    {
        if(onTimerCb_)
        {
            *onTimerCb_ = cb;
        }
    }
    void onTimer()
    {
        if(onTimerCb_ && *onTimerCb_)
        {
            (*onTimerCb_)();
        }
    }
    bool start(uint64_t timeoutUs, bool bstartOnce = true)
    {
        bstartOnce_ = bstartOnce;
        if(bstarted_)
        {
            stop();
        }
        esp_err_t err;
        if(bstartOnce_)
        {
            err = esp_timer_start_once(timerHandle_, timeoutUs);
        }
        else
        {
            err = esp_timer_start_periodic(timerHandle_, timeoutUs);
        }
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        bstarted_ = true;
        return true;
    }
    bool stop()
    {
        if(!bstarted_)
        {
            return true;
        }
        esp_err_t err = esp_timer_stop(timerHandle_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        bstarted_ = false;
        return true;
    }
    bool getPeriod(uint64_t *period)
    {
        if(!bstarted_ || bstartOnce_)
        {
            *period = 0;
            return true;
        }
        esp_err_t err = esp_timer_get_period(timerHandle_, period);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool getExpiryTime(uint64_t *expiry)
    {
        if(!bstarted_ || !bstartOnce_)
        {
            *expiry = 0;
            return true;
        }
        esp_err_t err = esp_timer_get_expiry_time(timerHandle_, expiry);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool isActive()
    {
        return esp_timer_is_active(timerHandle_);
    }
    void isrDispatchNeedYield()
    {
        #ifdef CONFIG_ESP_TIMER_SUPPORTS_ISR_DISPATCH_METHOD
        if(timerArgs_->dispatch_method == ESP_TIMER_ISR)
        {
            esp_timer_isr_dispatch_need_yield();
        }
        #endif
    } 
    static int64_t getBootTime()
    {
        return esp_timer_get_time();
    }
    static int64_t getNextAlarm()
    {
        return esp_timer_get_next_alarm();
    }
    static int64_t getNextAlarmForWakeUp()
    {
        return esp_timer_get_next_alarm_for_wake_up();
    }
    
private:
    bool init(const char* name, esp_timer_dispatch_t dispatchMethod, bool skipUnhandledEvents)
    {
        if(!timerArgs_)
        {
            return false;
        }

        timerArgs_->name = name;
        timerArgs_->dispatch_method = dispatchMethod;
        timerArgs_->skip_unhandled_events = skipUnhandledEvents;
        timerArgs_->callback = [](void* arg){
            MEspTimer* ptimer = reinterpret_cast<MEspTimer*>(arg);
            ptimer->onTimer();
        };
        timerArgs_->arg = this;
        esp_err_t err = esp_timer_create(timerArgs_, &timerHandle_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool deinit()
    {
        esp_err_t err = esp_timer_delete(timerHandle_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
private:
    bool bstartOnce_;
    bool bstarted_;
    esp_timer_handle_t timerHandle_;
    esp_timer_create_args_t* timerArgs_;
    EvnetOnTimerCallback* onTimerCb_;
};