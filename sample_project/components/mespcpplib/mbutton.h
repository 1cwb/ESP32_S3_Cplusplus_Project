#pragma once

#include "mgpio.h"
#include "mesptimer.h"
#include "meventhandler.h"
#include <list>
#include <mutex>
#include <functional>

#define TICKS_INTERVAL   5  //1tick = 5ms
#define DEBOUNCE_TICKS   2  //2tick
#define SHORT_TICKS      20 // 120/5 short press time = 120ms
#define LONG_TICKS       300 //1500ms/5
#define SERIAL_TICKS     4   //20 ms for long press hold


using EventButtonPressCb = std::function<void(uint32_t, bool, bool, uint32_t)>;

struct stButtonInfo
{
    uint32_t gpioNum : 7;
    bool blongPress : 1;
    uint32_t timer : 23;
    bool bbuttonRelease : 1;
    void* ptr;
};

enum eButtonType {
    BUTTON_TYPE_GPIO,
    BUTTON_TYPE_ADC,
    BUTTON_TYPE_CUSTOM
};

class MNewBuuton
{
public:
    MNewBuuton(gpio_num_t pin, uint16_t longPressTicks, uint16_t shortPressTicks ,uint8_t activeLevel = 0)
    :   gpio_(new MGpio(pin, GPIO_MODE_INPUT, activeLevel ? GPIO_PULLUP_DISABLE : GPIO_PULLUP_ENABLE, activeLevel ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE, GPIO_INTR_DISABLE)),
        cb_(new EventButtonPressCb),
        ticks_(0),
        longPressTicks_(longPressTicks),
        shortPressTicks_(shortPressTicks),
        longPressHoldCnt_(0),
        state_(0),
        debounceCnt_(0),
        activeLevel_(activeLevel),
        buttonLevel_(!activeLevel),
        type_(BUTTON_TYPE_GPIO)
    {

    }
    ~MNewBuuton()
    {
        if(cb_)
        {
            delete cb_;
            cb_ = nullptr;
        }
        if(gpio_)
        {
            delete gpio_;
            gpio_ = nullptr;
        }
    }
    MNewBuuton(const MNewBuuton&) = delete;
    MNewBuuton(MNewBuuton&&) = delete;
    MNewBuuton& operator=(const MNewBuuton& ) = delete;
    MNewBuuton& operator=(MNewBuuton&& ) = delete;
    MGpio* getGpio() const
    {
        return gpio_;
    }
    uint16_t getTicks() const {return ticks_;}
    uint16_t getLongPressTicks() const {return longPressTicks_;}
    uint16_t getShortPressTicks() const {return shortPressTicks_;}
    uint16_t getLongPressHoldCnt() const {return longPressHoldCnt_;}
    uint8_t  getState() const {return state_;}
    uint16_t getDebounceCnt() const {return debounceCnt_;}
    uint16_t getActiveLevel() const {return activeLevel_;}
    uint16_t getButtonLevel() const {return buttonLevel_;}
    eButtonType getType() const {return type_;}

    void setTicks(uint16_t ticks) {ticks_ = ticks;}
    void setState(uint8_t state) {state_ = state;}
    void setButtonLevel(uint8_t level) {buttonLevel_ = level;}
    void setDebounceCnt(uint16_t count) {debounceCnt_ = count;}
    void setType(eButtonType type) {type_ = type;}
    void setLongPressHoldCnt(uint16_t count) {longPressHoldCnt_ = count;}

    void registerEventCb(const EventButtonPressCb& cb)
    {
        if(cb_)
        {
            *cb_ = std::move(cb);
        }
    }
    void runCb(uint32_t buttonNum, bool blongPress, bool brelease, uint32_t time)
    {
        if(cb_ && *cb_)
        {
            (*cb_)(buttonNum, blongPress, brelease, time);
        }
    }
private:
    MGpio* gpio_;
    EventButtonPressCb* cb_;

    uint16_t ticks_;
    uint16_t longPressTicks_;
    uint16_t shortPressTicks_;
    uint16_t longPressHoldCnt_;
    uint8_t  state_ : 3;
    uint8_t  debounceCnt_ : 3;
    uint8_t  activeLevel_ : 1;
    uint8_t  buttonLevel_ : 1;
    eButtonType  type_;
};

class MButtonEventHandler : public eventClient
{
public:
    static MButtonEventHandler* getInstance()
    {
        static MButtonEventHandler buttonHandler_;
        return &buttonHandler_;
    }
    void addButton(MNewBuuton* button)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for(auto& it : mbuttonList_)
        {
            if(it == button)
            {
                printf("Button already added\n");
                return;
            }
        }
        mbuttonList_.emplace_back(button);
        if(!mbuttonList_.empty() && !timer_->isActive())
        {
            timer_->start(5000,false);
        }
    }
    void removeButton(MNewBuuton* button)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for(auto it = mbuttonList_.begin(); it != mbuttonList_.end(); ++it)
        {
            if((*it) == button)
            {
                mbuttonList_.erase(it);
                return;
            }
        }
        if(mbuttonList_.empty() && timer_->isActive())
        {
            timer_->stop();
        }
    }
private:
    MButtonEventHandler() : timer_(new MEspTimer("bttimer"))
    {
        enableEvent(E_EVENT_ID_BUTTON);
        MeventHandler::getINstance()->registerClient(this);
        timer_->registerOnTimerCallback([this](){
            stMsgData keyEvent;
            keyEvent.eventId = E_EVENT_ID_BUTTON;
            keyEvent.dataLen = 4;
            std::lock_guard<std::mutex> lock(mutex_);
            for(auto& it : mbuttonList_)
            {
                uint8_t gpioLevel = it->getGpio()->getLevel();
                if(it->getState() > 0)
                {
                    it->setTicks(it->getTicks()+1);
                }
                if(gpioLevel != it->getButtonLevel())
                {
                    it->setDebounceCnt(it->getDebounceCnt()+1);
                    if(it->getDebounceCnt() >= DEBOUNCE_TICKS)//2*5=10ms
                    {
                        it->setButtonLevel(gpioLevel);
                        it->setDebounceCnt(0);
                    }
                }
                else
                {
                    it->setDebounceCnt(0);
                }

                switch (it->getState())
                {
                case 0:
                    if(it->getButtonLevel() == it->getActiveLevel())
                    {
                        //button press down
                        stButtonInfo* pbtinfo = reinterpret_cast<stButtonInfo*>(malloc(sizeof(stButtonInfo)));
                        keyEvent.data = reinterpret_cast<uint8_t*>(pbtinfo);
                        pbtinfo->gpioNum = it->getGpio()->getPin();
                        pbtinfo->blongPress = false;
                        pbtinfo->timer = it->getShortPressTicks();
                        pbtinfo->bbuttonRelease = false;
                        pbtinfo->timer = 0;
                        pbtinfo->ptr = reinterpret_cast<void*>(it);
                        keyEvent.clean = [pbtinfo](void* p){
                            if(pbtinfo)
                            {
                                free(pbtinfo);
                            }
                        };
                        xQueueSend(MeventHandler::getINstance()->getQueueHandle(), &keyEvent, 20);
                        it->setTicks(0);
                        it->setState(1);
                    }
                    else
                    {
                        //no button press
                    }
                    break;
                case 1:
                    if(it->getButtonLevel() != it->getActiveLevel())
                    {
                        //button release
                        stButtonInfo* pbtinfo = reinterpret_cast<stButtonInfo*>(malloc(sizeof(stButtonInfo)));
                        keyEvent.data = reinterpret_cast<uint8_t*>(pbtinfo);
                        pbtinfo->gpioNum = it->getGpio()->getPin();
                        pbtinfo->blongPress = false;
                        pbtinfo->bbuttonRelease = true;
                        pbtinfo->timer = it->getTicks()*TICKS_INTERVAL;
                        pbtinfo->ptr = reinterpret_cast<void*>(it);
                        keyEvent.clean = [pbtinfo](void* p){
                            if(pbtinfo)
                            {
                                free(pbtinfo);
                            }
                        };
                        xQueueSend(MeventHandler::getINstance()->getQueueHandle(), &keyEvent, 20);
                        it->setTicks(0);
                        it->setState(0);
                    }
                    else if(it->getTicks() > it->getLongPressTicks())
                    {
                        //button long press start
                        stButtonInfo* pbtinfo = reinterpret_cast<stButtonInfo*>(malloc(sizeof(stButtonInfo)));
                        keyEvent.data = reinterpret_cast<uint8_t*>(pbtinfo);
                        pbtinfo->gpioNum = it->getGpio()->getPin();
                        pbtinfo->blongPress = true;
                        pbtinfo->timer = it->getLongPressHoldCnt()*TICKS_INTERVAL;
                        pbtinfo->bbuttonRelease = false;
                        pbtinfo->ptr = reinterpret_cast<void*>(it);
                        keyEvent.clean = [pbtinfo](void* p){
                            if(pbtinfo)
                            {
                                free(pbtinfo);
                            }
                        };
                        xQueueSend(MeventHandler::getINstance()->getQueueHandle(), &keyEvent, 20);
                        it->setState(5);
                    }
                    break;
                    case 5:
                        if(it->getButtonLevel() == it->getActiveLevel())
                        {
                            if(it->getTicks() >= (it->getLongPressHoldCnt()+1)*4)
                            {
                                //long press hold
                                it->setLongPressHoldCnt(it->getLongPressHoldCnt()+1);

                                stButtonInfo* pbtinfo = reinterpret_cast<stButtonInfo*>(malloc(sizeof(stButtonInfo)));
                                keyEvent.data = reinterpret_cast<uint8_t*>(pbtinfo);
                                pbtinfo->gpioNum = it->getGpio()->getPin();
                                pbtinfo->blongPress = true;
                                pbtinfo->timer = it->getLongPressHoldCnt()*TICKS_INTERVAL*4;
                                pbtinfo->bbuttonRelease = false;
                                pbtinfo->ptr = reinterpret_cast<void*>(it);
                                keyEvent.clean = [pbtinfo](void* p){
                                    if(pbtinfo)
                                    {
                                        free(pbtinfo);
                                    }
                                };
                                xQueueSend(MeventHandler::getINstance()->getQueueHandle(), &keyEvent, 20);
                            }
                        }
                        else
                        {
                            //long press release
                            stButtonInfo* pbtinfo = reinterpret_cast<stButtonInfo*>(malloc(sizeof(stButtonInfo)));
                            keyEvent.data = reinterpret_cast<uint8_t*>(pbtinfo);
                            pbtinfo->gpioNum = it->getGpio()->getPin();
                            pbtinfo->blongPress = true;
                            pbtinfo->timer = it->getLongPressHoldCnt()*TICKS_INTERVAL*4;
                            pbtinfo->bbuttonRelease = true;
                            pbtinfo->ptr = reinterpret_cast<void*>(it);
                            keyEvent.clean = [pbtinfo](void* p){
                                if(pbtinfo)
                                {
                                    free(pbtinfo);
                                }
                            };
                            xQueueSend(MeventHandler::getINstance()->getQueueHandle(), &keyEvent, 20);
                            it->setState(0);
                            it->setTicks(0);
                            it->setLongPressHoldCnt(0);
                        }
                        break;
                default:
                    break;
                }
            }
        });
    }
    ~MButtonEventHandler()
    {
        disableEvent(E_EVENT_ID_KEY);
        MeventHandler::getINstance()->unregisterClient(this);
        if(timer_)
        {
            if(timer_->isActive())
            {
                timer_->stop();
            }
            delete timer_;
            timer_ = nullptr;
        }
    }
    MButtonEventHandler(const MButtonEventHandler&) = delete;
    MButtonEventHandler(MNewBuuton&&) = delete;
    MButtonEventHandler& operator=(const MButtonEventHandler& ) = delete;
    MButtonEventHandler& operator=(MButtonEventHandler&& ) = delete;
    virtual void onEvent(uint32_t eventId, uint8_t* data ,uint32_t dataLen)
    {
        if(!data)
        {
            return;
        }
        if(eventId & E_EVENT_ID_BUTTON)
        {
            stButtonInfo* btinfo = reinterpret_cast<stButtonInfo*>(data);
            MNewBuuton* bt = reinterpret_cast<MNewBuuton*>(btinfo->ptr);

            if(bt)
            {
                //printf("button %d prees, blongPress(%d),brelease(%d)holdtimer(%d)\n", btinfo->gpioNum, btinfo->blongPress, btinfo->bbuttonRelease ,btinfo->timer);
                bt->runCb(btinfo->gpioNum, btinfo->blongPress, btinfo->bbuttonRelease ,btinfo->timer);
            }
        }
    }
private:
    MEspTimer* timer_;
    std::list<MNewBuuton*> mbuttonList_;
    std::mutex mutex_;
};

class MButton
{
public:
    MButton(gpio_num_t pin, uint16_t longPressTicks = 300, uint16_t shortPressTicks = 36, uint8_t activeLevel = 0) : button_ (new MNewBuuton(pin, longPressTicks, shortPressTicks, activeLevel))
    {
        MButtonEventHandler::getInstance()->addButton(button_);
    }
    ~MButton()
    {
        MButtonEventHandler::getInstance()->removeButton(button_);
        if(button_)
        {
            delete button_;
            button_ = nullptr;
        }
    }
    MButton(const MButton&) = delete;
    MButton(MButton&&) = delete;
    MButton& operator=(const MButton& ) = delete;
    MButton& operator=(MButton&& ) = delete;
    void registerEventCb(const EventButtonPressCb& cb)
    {
        if(button_)
        {
            button_->registerEventCb(cb);
        }
    }
    uint16_t getPinNum() const {return button_->getGpio()->getPin();}
private:
    MNewBuuton* button_;
};