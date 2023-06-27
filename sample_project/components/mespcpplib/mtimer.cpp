#include "mtimer.h"

//    gptimer_handle_t handle_;
//    gptimer_config_t config_;
MTimer::MTimer():started_(false),handle_(nullptr)
{

}
MTimer::~MTimer()
{

}
bool MTimer::init(uint32_t resolutionHz, gptimer_count_direction_t direction, gptimer_clock_source_t clkSrc, bool bIntrShared)
{
    config_.clk_src = clkSrc;
    config_.direction = direction;
    config_.resolution_hz = resolutionHz;
    config_.flags.intr_shared = (bIntrShared ? 1 : 0);
    esp_err_t err = gptimer_new_timer(&config_, &handle_);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}
bool MTimer::deinit()
{
    if(handle_)
    {
        stop();
        disable();
        esp_err_t err = gptimer_del_timer(handle_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        handle_ = nullptr;
        return true;
    }
    return true;
}
bool MTimer::enable()
{
    esp_err_t err = gptimer_enable(handle_);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}
bool MTimer::disable()
{
    esp_err_t err = gptimer_disable(handle_);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}
bool MTimer::start()
{
    esp_err_t err =gptimer_start(handle_);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    started_ = true;
    return true;
}
bool MTimer::stop()
{
    esp_err_t err =gptimer_stop(handle_);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    started_ = false;
    return true;
}
bool MTimer::setRawCount(const uint64_t value)
{
    esp_err_t err = gptimer_set_raw_count(handle_, value);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}
bool MTimer::getRawCount(uint64_t* value)
{
    esp_err_t err = gptimer_get_raw_count(handle_, value);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}
bool MTimer::enableAlarm(uint64_t alarmCount, uint64_t reloadCount, bool autoRealoadOnAlarm)
{
    alarmConfig_.alarm_count = alarmCount;
    alarmConfig_.reload_count = reloadCount;
    alarmConfig_.flags.auto_reload_on_alarm = (autoRealoadOnAlarm ? 1 : 0);
    esp_err_t err = gptimer_set_alarm_action(handle_, &alarmConfig_);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}
bool MTimer::updateAlarmValue(uint64_t alarmCount)
{
    alarmConfig_.alarm_count = alarmCount;
    esp_err_t err = gptimer_set_alarm_action(handle_, &alarmConfig_);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}
bool MTimer::disableAlarm()
{
    esp_err_t err = gptimer_set_alarm_action(handle_, nullptr);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}

bool MTimer::registerEventCallbacks(const gptimer_alarm_cb_t cbs, void *userData)
{
    if(cbs == nullptr)
    {
        printf("Error: %s()%d %s",__FUNCTION__,__LINE__,"Invalied param! ");
        return false;
    }
    cbs_.on_alarm = cbs;
    esp_err_t err = gptimer_register_event_callbacks(handle_, &cbs_, userData);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}
