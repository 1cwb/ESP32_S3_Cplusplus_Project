#pragma once
#include "esp_timer.h"
#include "sdkconfig.h"
#include "mgpio.h"
#include "meventhandler.h"
#include "mtimer.h"

struct stButtonInfo
{
    uint32_t gpioNum : 7;
    bool blongPress : 1;
    uint32_t timer : 24;
    static void parseBttonInfo(uint32_t val, uint32_t* buttonNum, bool* longPress, uint32_t* timer)
    {
        stButtonInfo* pinfo = reinterpret_cast<stButtonInfo*>(&val);
        *buttonNum = pinfo->gpioNum;
        *longPress = pinfo->blongPress;
        *timer = pinfo->timer;
    }
};

class MprivButton
{
public:
    MprivButton(gpio_num_t pin,  bool loopKeyEvent, uint32_t longPressTimer, uint32_t loopPressTimer, bool bInitHighLevel)
    : preTickCount_(0),
      totalTickCount_(0),
      preLongPressTickCount_(0),
      bloopKeyEvent_(loopKeyEvent),
      longPressTimer_(longPressTimer),
      loopPressTimer_(loopPressTimer),
      blongPress_(false),
      button_(new MGpio(pin, GPIO_MODE_INPUT, bInitHighLevel ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE, bInitHighLevel ? GPIO_PULLDOWN_DISABLE : GPIO_PULLDOWN_ENABLE, GPIO_INTR_ANYEDGE))
    {
        button_->installIsrService();
        enable();
    }
    ~MprivButton()
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
    uint32_t getPreLongPressTickCount() const
    {
        return preLongPressTickCount_;
    }
    void setPreLongPressTickCount(uint32_t tick)
    {
        preLongPressTickCount_ = tick;
    }
    void clearPreLongPressTickCount()
    {
        preLongPressTickCount_ = 0;
    }
    uint32_t getLongPressTimer() const
    {
        return longPressTimer_;
    }
    uint32_t getLoopPressTimer() const
    {
        return loopPressTimer_;
    }
    bool bloopKeyEvent()
    {
        return bloopKeyEvent_;
    }
    void setLongPress(bool blongPress)
    {
        blongPress_ = blongPress;
    }
    bool bLongPress() const
    {
        return blongPress_;
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
            reinterpret_cast<MprivButton*>(pa)->setPreticCount(xTaskGetTickCount());
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
    uint32_t preLongPressTickCount_;
    bool bloopKeyEvent_;
    uint32_t longPressTimer_;
    uint32_t loopPressTimer_;
    bool blongPress_;
    MGpio* button_;
};

class MprivButtonParse : public eventClient
{
public:
    static MprivButtonParse* getINstance()
    {
        static MprivButtonParse butonHnadle;
        return &butonHnadle;
    }
    virtual void onEvent(uint32_t eventId, uint8_t* data ,uint32_t dataLen)
    {
        if(!data)
        {
            return ;
        }
        if(eventId & E_EVENT_ID_KEY)
        {
            if(!timer_.started())
            {
                timer_.start();
            }
            MprivButton* pbutton = reinterpret_cast<MprivButton*>(data);
            if(pbutton->getGpio()->getLevel() == 0)
            {
                pbutton->setRemeTotalTickCount(xTaskGetTickCount() - pbutton->getPreticCount());
                if(pbutton->bloopKeyEvent())
                {
                    if(pbutton->getTotalTickCount() >= pbutton->getLongPressTimer() + pbutton->getPreLongPressTickCount())
                    {
                        stMsgData keyEvent;
                        keyEvent.eventId = E_EVENT_ID_BUTTON;
                        keyEvent.dataLen = 4;
                        keyEvent.val = 0;
                        stButtonInfo* pbtinfo = reinterpret_cast<stButtonInfo*>(&keyEvent.val);
                        pbutton->setLongPress(true);
                        pbtinfo->gpioNum = pbutton->getGpio()->getPin();
                        pbtinfo->blongPress = pbutton->bLongPress();
                        pbtinfo->timer = pbutton->getTotalTickCount();
                        xQueueSend(MeventHandler::getINstance()->getQueueHandle(), &keyEvent, 20);
                        pbutton->setPreLongPressTickCount(pbutton->getTotalTickCount() - pbutton->getLongPressTimer() + pbutton->getLoopPressTimer());
                    }
                }
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
                    pbtinfo->blongPress = pbutton->bLongPress();
                    pbtinfo->timer = pbutton->getTotalTickCount();
                    pbutton->setLongPress(false);
                    xQueueSend(MeventHandler::getINstance()->getQueueHandle(), &keyEvent, 20);
                    pbutton->clearTotalTickCount();
                    pbutton->clearPreLongPressTickCount();
                }
            }
            pbutton->setPreticCount(xTaskGetTickCount());
        }
    }
    void addButton(MprivButton* bt)
    {
        for(auto& it : buttonList_)
        {
            if(it == bt)
            {
                printf("the button already added\n");
                return ;
            }
        }
        buttonList_.emplace_back(bt);
    }
    void removeButton(MprivButton* bt)
    {
        for(auto it = buttonList_.begin(); it != buttonList_.end(); ++it)
        {
            if(*it == bt)
            {
                buttonList_.erase(it);
                return ;
            }
        }
    }
    void removeAllButton()
    {
        buttonList_.clear();
    }
    std::list<MprivButton*>* getButtonList()
    {
        return &buttonList_;
    }
    MTimer* getTimer()
    {
        return &timer_;
    }
private:
    MprivButtonParse()
    {
        enableEvent(E_EVENT_ID_KEY);
        MeventHandler::getINstance()->registerClient(this);
        timer_.init(1000000);//1mhz 1us/tick
        timer_.registerEventCallbacks([](gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)->bool{
            MprivButtonParse* pthis = reinterpret_cast<MprivButtonParse*>(user_ctx);
            auto it = pthis->getButtonList()->begin();
            for(; it != pthis->getButtonList()->end(); it++)
            {
                if((*it)->getGpio()->getLevel() == 0)
                {
                    BaseType_t xTaskWokenByReceive = pdFALSE;
                    stMsgData keyEvent;
                    keyEvent.eventId = E_EVENT_ID_KEY;
                    keyEvent.dataLen = 4;
                    keyEvent.data = reinterpret_cast<uint8_t*>(*it);
                    xQueueSendFromISR(MeventHandler::getINstance()->getQueueHandle(), &keyEvent, &xTaskWokenByReceive);
                }
            }
            if(it == pthis->getButtonList()->end())
            {
                pthis->getTimer()->stop();
            }
            return true;
        }, this);
        timer_.enableAlarm(10000, 0, true);//10ms
        timer_.enable();
    }
    virtual ~MprivButtonParse()
    {
        MeventHandler::getINstance()->unregisterClient(this);
        timer_.stop();
        timer_.disableAlarm();
        timer_.disable();
        timer_.deinit();
    }
private:
    std::list<MprivButton*> buttonList_;
    MTimer timer_;
};

class MButton : public eventClient
{
    using EventButtonPressCb = std::function<void(uint32_t, uint32_t, uint32_t, bool, uint32_t)>;
public:
    MButton(gpio_num_t pin,  bool loopKeyEvent = true,uint32_t longPressTimer = 100, uint32_t loopPressTimer = 20, bool bInitHighLevel = true):button_(new MprivButton(pin, loopKeyEvent, longPressTimer, loopPressTimer, bInitHighLevel)),keyCb_(new EventButtonPressCb)
    {
        enableEvent(E_EVENT_ID_BUTTON);
        MeventHandler::getINstance()->registerClient(this);
        MprivButtonParse::getINstance()->addButton(button_);
    }
    virtual ~MButton()
    {
        MprivButtonParse::getINstance()->removeButton(button_);
        MeventHandler::getINstance()->unregisterClient(this);
        if(button_)
        {
            delete button_;
            button_ = nullptr;
        }
        if(keyCb_)
        {
            delete keyCb_;
            keyCb_ = nullptr;
        }
    }
    virtual void onEvent(uint32_t eventId, uint8_t* data ,uint32_t dataLen)
    {
        if(!data)
        {
            return;
        }
        if(eventId & E_EVENT_ID_BUTTON)
        {
            uint32_t buttonNum = 0;
            bool longPress = 0;
            uint32_t timernum = 0;
            stButtonInfo::parseBttonInfo(reinterpret_cast<uint32_t>(data), &buttonNum, &longPress, &timernum);

            if(button_->getGpio()->getPin() == buttonNum)
            {
                if(keyCb_ && *keyCb_)
                {
                    (*keyCb_)(eventId & E_EVENT_ID_BUTTON, buttonNum, dataLen, longPress, timernum);
                }
            }
        }
    }
    void setButtonPressCb(const EventButtonPressCb&& keycb)
    {
        *keyCb_ = keycb;
    }
private:
   MprivButton* button_; 
   EventButtonPressCb* keyCb_;
};