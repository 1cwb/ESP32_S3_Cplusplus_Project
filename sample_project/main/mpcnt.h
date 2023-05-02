#pragma once
#include <atomic>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/pulse_cnt.h"
#include <functional>
#include <list>

struct stEncoderData
{
    pcnt_watch_event_data_t *edata;
    pcnt_unit_handle_t phandle;
};

class MPcntUnit
{
public:
    MPcntUnit(int32_t highLimit = 1000, int32_t lowLimit = -1000) : pcntHanlde_(nullptr)
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
    bool enable()
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
    bool disable()
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
    bool start()
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
    bool stop()
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
    int32_t getHighLimit() const
    {
        return pcntUnitConfig_.high_limit;
    }
    int32_t getLowLimit() const
    {
        return pcntUnitConfig_.low_limit;
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
    int getCount()
    {
        int val = 0;
        getCount(&val);
        return val;
    }
    pcnt_unit_handle_t getPcntUnitHand()
    {
        return pcntHanlde_;
    }
    bool AddWatchPoint(int32_t watchPoint)
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
    bool RemoveWatchPoint(int32_t watchPoint)
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
    bool RegisterEventCb(const pcnt_event_callbacks_t *cbs, void *user_data)
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
        pcnt_watch_cb_t onReach = [](pcnt_unit_handle_t unit, const pcnt_watch_event_data_t *edata, void *user_ctx)->bool{
            stEncoderData* ecData = static_cast<stEncoderData*>(malloc(sizeof(stEncoderData)));
            ecData->edata = static_cast<pcnt_watch_event_data_t *>(malloc(sizeof(pcnt_watch_event_data_t)));
            ecData->phandle = unit;
            memcpy(ecData->edata, edata, sizeof(pcnt_watch_event_data_t));
            BaseType_t xTaskWokenByReceive = pdFALSE;
            stMsgData encoder;
            encoder.eventId = E_EVENT_ID_ENCODER;
            encoder.dataLen = 4;
            encoder.data = reinterpret_cast<uint8_t*>(ecData);
            encoder.clean = [&](){
                if(ecData)
                {
                    printf("clean encoder now\n");
                    if(ecData->edata)
                    {
                        free(ecData->edata);
                        ecData->edata = nullptr;
                    }
                    free(ecData);
                    ecData = nullptr;
                }
            };
            xQueueSendFromISR(MeventHandler::getINstance()->getQueueHandle(), &encoder, &xTaskWokenByReceive);
            return true;
        };
        const pcnt_event_callbacks_t cbs = {onReach};
        err = pcnt_unit_register_event_callbacks(pcntHanlde_, &cbs, nullptr);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
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

class Mencoder
{
public:
    Mencoder():unit(new MPcntUnit),cha(new MPcntChannel(unit)),chb(new MPcntChannel(unit))
    {
        //unit.RegisterEventCb();
    }
    bool init(int32_t gpioa, int32_t gpiob, const uint32_t glitchNS = 1000)
    {
        unit->setGlitchFilter(glitchNS);
        cha->init(gpioa,gpiob);
        chb->init(gpiob,gpioa);
        
        cha->setEdgeAction(PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE);
        chb->setEdgeAction(PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE);

        cha->setLevelAction(PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
        chb->setLevelAction(PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
        return true;
    }
    bool addWatchPoint(int32_t watch_points[], uint16_t size)
    {
        for(uint16_t s = 0; s < size; s++)
        {
            if(!unit->AddWatchPoint(watch_points[s]))
            {
                return false;
            }
        }
        return true;
    }
    bool addWatchPoint(int32_t watchPoint)
    {

        if(!unit->AddWatchPoint(watchPoint))
        {
            return false;
        }
        return true;
    }
    bool start()
    {
        unit->enable();
        unit->clearCount();
        unit->start();
        return true;
    }
    bool stop()
    {
        unit->clearCount();
        unit->stop();
        unit->disable();
        return true;
    }
    ~Mencoder()
    {
        if(cha)
        {
            delete cha;
        }
        if(chb)
        {
            delete chb;
        }
        if(unit)
        {
            unit->stop();
            unit->disable();
            delete unit;
        }
    }
    Mencoder(const Mencoder&) = delete;
    Mencoder(Mencoder&&) = delete;
    Mencoder& operator=(const Mencoder& ) = delete;
    Mencoder& operator=(Mencoder&& ) = delete;
    MPcntUnit* getUnit()
    {
        return unit;
    }
private:
    MPcntUnit* unit;
    MPcntChannel* cha;
    MPcntChannel* chb;
};

class MEncoderParse : public eventClient
{
    using EncoderCb = std::function<void(pcnt_unit_handle_t, pcnt_watch_event_data_t*)>;
public:
    MEncoderParse()
    {
        enableEvent(E_EVENT_ID_ENCODER);
        MeventHandler::getINstance()->registerClient(this);
    }
    virtual ~MEncoderParse()
    {
        MeventHandler::getINstance()->unregisterClient(this);
    }
    virtual void onEvent(uint32_t eventId, uint8_t* data ,uint32_t dataLen)
    {
        if(!data)
        {
            return ;
        }
        if(eventId & E_EVENT_ID_ENCODER)
        {
            stEncoderData* pencoder = reinterpret_cast<stEncoderData*>(data);
            pcnt_unit_handle_t mhandle = pencoder->phandle;
            pcnt_watch_event_data_t *watchEvData = pencoder->edata;
            if(encoderCb_)
            {
                encoderCb_(mhandle, watchEvData);
            }
        }
    }
    void setEncoderCb(const EncoderCb & cb)
    {
        encoderCb_ = cb;
    }
private:
    EncoderCb encoderCb_;
};
