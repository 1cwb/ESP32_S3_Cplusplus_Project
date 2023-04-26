#pragma once
#include "esp_timer.h"
#include "sdkconfig.h"
#include "mgpio.h"
#include "meventhandler.h"

struct stButtonInfo
{
    uint32_t gpioNum : 7;
    bool pressDown : 1;
    static void parseBttonInfo(uint32_t val, uint32_t* buttonNum, bool* bpressDown)
    {
        stButtonInfo* pinfo = reinterpret_cast<stButtonInfo*>(&val);
        *buttonNum = pinfo->gpioNum;
        *bpressDown = pinfo->pressDown;
    }
};

class MButton
{
public:
    MButton(gpio_num_t pin, bool bInitHighLevel = true)
    : preTickCount_(0),
      totalTickCount_(0),
      button_(new MGpio(pin, GPIO_MODE_INPUT, bInitHighLevel ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE, bInitHighLevel ? GPIO_PULLDOWN_DISABLE : GPIO_PULLDOWN_ENABLE, GPIO_INTR_ANYEDGE))
    {
        button_->installIsrService();
        enable();
    }
    ~MButton()
    {
        //button_.uninstallIsrService();

        unRegisterButtonHandler();
        disable();
        if(button_)
        {
            delete button_;
        }
    }
    bool enable()
    {
        registerButtonHandler();
        if(!button_->intrEnable())
        {
            return false;   
        }
        return true;
    }
    bool disable()
    {
        if(!button_->intrDisable())
        {
            return false;
        }
        return true;
    }
    void setPreticCount(uint32_t tick)
    {
        preTickCount_ = tick;
    }
    uint32_t getPreticCount() const
    {
        return preTickCount_;
    }
    void setRemeTotalTickCount(uint32_t tick)
    {
        totalTickCount_ += tick;
    }
    void clearTotalTickCount()
    {
        totalTickCount_ = 0;
    }
    uint32_t getTotalTickCount() const
    {
        return totalTickCount_;
    }
    MGpio* getGpio()
    {
        return button_;
    }
private: 
    void registerButtonHandler()
    {
        button_->addIsrHandler([](void* pa){
            if(!pa)
            {
                return;
            }
            BaseType_t xTaskWokenByReceive = pdFALSE;
            stMsgData keyEvent;
            keyEvent.eventId = E_EVENT_ID_KEY;
            keyEvent.dataLen = 4;
            keyEvent.data = static_cast<uint8_t*>(pa);
            xQueueSendFromISR(MeventHandler::getINstance()->getQueueHandle(), &keyEvent, &xTaskWokenByReceive);
        },this);
    }
    void unRegisterButtonHandler()
    {
        button_->removeIsrHandler();
    }
private:
    uint32_t preTickCount_;
    uint32_t totalTickCount_;
    MGpio* button_;
};

class MbuttonParse : public eventClient
{
public:
    MbuttonParse()
    {
        enableEvent(E_EVENT_ID_KEY);
        MeventHandler::getINstance()->registerClient(this);
    }
    virtual ~MbuttonParse()
    {
        MeventHandler::getINstance()->unregisterClient(this);
    }
    virtual void onEvent(uint32_t eventId, uint8_t* data ,uint32_t dataLen)
    {
        if(!data)
        {
            return ;
        }
        if(eventId & E_EVENT_ID_KEY)
        {
            MButton* pbutton = reinterpret_cast<MButton*>(data);
            if(pbutton->getGpio()->getLevel() == 0)
            {
                pbutton->setRemeTotalTickCount(xTaskGetTickCount() - pbutton->getPreticCount());
            }
            else
            {
                if(pbutton->getTotalTickCount() > 5)
                {
                    stMsgData keyEvent;
                    keyEvent.eventId = E_EVENT_ID_BUTTON;
                    keyEvent.dataLen = 4;
                    keyEvent.val = 0;
                    stButtonInfo* pbtinfo = reinterpret_cast<stButtonInfo*>(&keyEvent.val);
                    pbtinfo->gpioNum = pbutton->getGpio()->getPin();
                    pbtinfo->pressDown = true;
                    xQueueSend(MeventHandler::getINstance()->getQueueHandle(), &keyEvent, 20);
                    pbutton->clearTotalTickCount();
                }
            }
            pbutton->setPreticCount(xTaskGetTickCount());
        }
    }
};
