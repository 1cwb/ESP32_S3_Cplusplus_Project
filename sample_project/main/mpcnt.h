#pragma once
#include <atomic>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/pulse_cnt.h"
#include <list>

class MPcntUnit
{
public:
    MPcntUnit(int32_t highLimit = 100, int32_t lowLimit = -100) : pcntHanlde_(nullptr)
    {
        init(highLimit, lowLimit);
    }
    ~MPcntUnit()
    {
        deinit();
    }
    MPcntUnit(const MPcntUnit&) = delete;
    MPcntUnit(MPcntUnit&&) = delete;
    MPcntUnit& operator=(const MPcntUnit& ) = delete;
    MPcntUnit& operator=(MPcntUnit&& ) = delete;
    bool pcntEnable()
    {
        if(!pcntHanlde_)
        {
            return false;
        }
        esp_err_t err = pcnt_unit_enable(pcntHanlde_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool pcntDisable()
    {
        if(!pcntHanlde_)
        {
            return false;
        }
        esp_err_t err = pcnt_unit_disable(pcntHanlde_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool pcntStart()
    {
        if(!pcntHanlde_)
        {
            return false;
        }
        esp_err_t err = pcnt_unit_start(pcntHanlde_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool pcntStop()
    {
        if(!pcntHanlde_)
        {
            return false;
        }
        esp_err_t err = pcnt_unit_stop(pcntHanlde_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool clearCount()
    {
        if(!pcntHanlde_)
        {
            return false;
        }
        esp_err_t err = pcnt_unit_clear_count(pcntHanlde_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool getCount(int* value)
    {
        if(!pcntHanlde_)
        {
            return false;
        }
        esp_err_t err = pcnt_unit_get_count(pcntHanlde_, value);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    pcnt_unit_handle_t getPcntUnitHand()
    {
        return pcntHanlde_;
    }
    bool unitAddWatchPoint(int32_t watchPoint)
    {
        if(!pcntHanlde_)
        {
            return false;
        }
        esp_err_t err = pcnt_unit_add_watch_point(pcntHanlde_, watchPoint);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool unitRemoveWatchPoint(int32_t watchPoint)
    {
        if(!pcntHanlde_)
        {
            return false;
        }
        esp_err_t err = pcnt_unit_remove_watch_point(pcntHanlde_, watchPoint);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    //观察点中断回调highLimit and lowLimit
    bool unitRegisterEventCb(const pcnt_event_callbacks_t *cbs, void *user_data)
    {
        if(!pcntHanlde_)
        {
            return false;
        }
        esp_err_t err = pcnt_unit_register_event_callbacks(pcntHanlde_, cbs, user_data);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    //毛刺滤波
    bool setGlitchFilter(const uint32_t glitchNS)
    {
        if(!pcntHanlde_)
        {
            return false;
        }
        pcnt_glitch_filter_config_t glitch = {glitchNS};
        esp_err_t err = pcnt_unit_set_glitch_filter(pcntHanlde_, &glitch);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool disableGlitchFilter()
    {
        if(!pcntHanlde_)
        {
            return false;
        }
        esp_err_t err = pcnt_unit_set_glitch_filter(pcntHanlde_, nullptr);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
private:
    bool init(int32_t highLimit, int32_t lowLimit)
    {
        if(pcntUnitCount_ >= SOC_PCNT_UNITS_PER_GROUP)
        {
            printf("Error: pcntUnit count can not > %d\n",SOC_PCNT_UNITS_PER_GROUP);
            return false;
        }

        pcntUnitConfig_.high_limit = highLimit;
        pcntUnitConfig_.low_limit = lowLimit;
        esp_err_t err = pcnt_new_unit(&pcntUnitConfig_, &pcntHanlde_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        pcntUnitCount_++;
        return true;
    }
    bool deinit()
    {
        esp_err_t err = pcnt_unit_disable(pcntHanlde_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        err = pcnt_del_unit(pcntHanlde_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        pcntUnitCount_--;
        return true;
    }
private:
    static std::atomic<uint8_t> pcntUnitCount_;
    pcnt_unit_handle_t pcntHanlde_;
    pcnt_unit_config_t pcntUnitConfig_;
};
std::atomic<uint8_t> MPcntUnit::pcntUnitCount_ = 0;


class MPcntChannel
{
public:
    MPcntChannel(MPcntUnit* unit) : unit_(unit), pcntChHandle_(nullptr)
    {
        if(!unit)
        {
            printf("Error : unit is nullptr\n");
        }
    }
    ~MPcntChannel()
    {

    }
    
    MPcntChannel(const MPcntChannel&) = delete;
    MPcntChannel(MPcntChannel&&) = delete;
    MPcntChannel& operator=(const MPcntChannel& ) = delete;
    MPcntChannel& operator=(MPcntChannel&& ) = delete;

    bool init(int32_t edgeGpioNum = -1, int32_t levelGpioNum = -1, bool invertEdgeInput = false, bool invertLevelInput = false, uint8_t virtEdgeIoLevel = 0, uint8_t virtLevelIoLevel = 0)
    {
        pcntChConfig_.edge_gpio_num = edgeGpioNum; 
        pcntChConfig_.level_gpio_num = levelGpioNum; 
        pcntChConfig_.flags.invert_edge_input = invertEdgeInput;  
        pcntChConfig_.flags.invert_level_input = invertLevelInput;  
        pcntChConfig_.flags.virt_edge_io_level = virtEdgeIoLevel;  
        pcntChConfig_.flags.virt_level_io_level = virtLevelIoLevel; 
        pcntChConfig_.flags.io_loop_back = 0;

        esp_err_t err = pcnt_new_channel(unit_->getPcntUnitHand(), &pcntChConfig_, &pcntChHandle_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }

    bool setEdgeAction(pcnt_channel_edge_action_t posAct, pcnt_channel_edge_action_t negAct)
    {
        if(!pcntChHandle_)
        {
            printf("Error: %s()%d return false Beacuse of nullptr\n",__FUNCTION__,__LINE__);
            return false;
        }
        esp_err_t err = pcnt_channel_set_edge_action(pcntChHandle_, posAct, negAct);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool setLevelAction(pcnt_channel_level_action_t highAct, pcnt_channel_level_action_t lowAct)
    {
        if(!pcntChHandle_)
        {
            printf("Error: %s()%d return false Beacuse of nullptr\n",__FUNCTION__,__LINE__);
            return false;
        }
        esp_err_t err = pcnt_channel_set_level_action(pcntChHandle_, highAct, lowAct);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool deinit()
    {
        if(pcntChHandle_)
        {
            esp_err_t err = pcnt_del_channel(pcntChHandle_);
            if(err != ESP_OK)
            {
                printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
                return false;
            }
        }
        return true;
    }

private:
    MPcntUnit* unit_;
    pcnt_channel_handle_t pcntChHandle_;
    pcnt_chan_config_t pcntChConfig_;
};
/*
class MPcnt
{
public:
    MPcnt() : pcntHanlde_(nullptr), pcntChHandle_(nullptr)
    {

    }
    ~MPcnt()
    {

    }
    MPcnt(const MPcnt&) = delete;
    MPcnt(MPcnt&&) = delete;
    MPcnt& operator=(const MPcnt& ) = delete;
    MPcnt& operator=(MPcnt&& ) = delete;

    bool pcntInit(int32_t edgeGpioNum = -1, int32_t levelGpioNum = -1, int32_t highLimit = 100, int32_t lowLimit = -100,
                    bool invertEdgeInput = false, bool invertLevelInput = false, uint8_t virtEdgeIoLevel = 0, uint8_t virtLevelIoLevel = 0)
    {
        pcntUnitConfig_.high_limit = highLimit;
        pcntUnitConfig_.low_limit = lowLimit;

        esp_err_t err = pcnt_new_unit(&pcntUnitConfig_, &pcntHanlde_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }

        pcntChConfig_.edge_gpio_num = edgeGpioNum; 
        pcntChConfig_.level_gpio_num = levelGpioNum; 
        pcntChConfig_.flags.invert_edge_input = invertEdgeInput;  
        pcntChConfig_.flags.invert_level_input = invertLevelInput;  
        pcntChConfig_.flags.virt_edge_io_level = virtEdgeIoLevel;  
        pcntChConfig_.flags.virt_level_io_level = virtLevelIoLevel; 
        pcntChConfig_.flags.io_loop_back = 0;

        err = pcnt_new_channel(pcntHanlde_, &pcntChConfig_, &pcntChHandle_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool pcntDeInit()
    {
        esp_err_t err;
        if(pcntHanlde_)
        {
            err =  pcnt_unit_disable(pcntHanlde_);
            if(err != ESP_OK)
            {
                printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
                return false;
            }
        }
        if()
    }
private:
    pcnt_unit_handle_t pcntHanlde_;
    pcnt_channel_handle_t pcntChHandle_;
    pcnt_unit_config_t pcntUnitConfig_;
    pcnt_chan_config_t pcntChConfig_;
};
*/