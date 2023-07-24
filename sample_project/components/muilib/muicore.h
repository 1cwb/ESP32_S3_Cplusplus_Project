#pragma once
#include <thread>
#include <chrono>
#include "muiinterface.h"
#include "font.h"
#include "mrgbcolor.h"
#include "muicommon.h"
#include "muimsgqueue.h"
#include "meventhandler.h"
#include "mbutton.h"
#include <list>

class MUicore
{
public:
    static MUicore*getInstance()
    {
        static MUicore core;
        return &core;
    }

    MUicore(const MUicore&) = delete;
    MUicore(MUicore&&) = delete;
    MUicore& operator=(const MUicore& ) = delete;
    MUicore& operator=(MUicore&& ) = delete;
    void addLcd(MUiInterface* lcd)
    {
        if(!lcd_)
        {
            lcd_ =  lcd;
        }
        lcdRamSize_ = lcd_->getWidth()*lcd_->getHeight()*lcd_->getPixelBytes();
    }
    uint8_t getPixelBytes() const
    {
        if(lcd_)
        {
            return lcd_->getPixelBytes();
        }
        return 0;
    }
    void addToUiCore(MUiWindBase* ui)
    {
        std::lock_guard<std::mutex> lock(windownListMutex_);
        for(auto& it : windownList_)
        {
            if(it == ui)
            {
                return;
            }
        }
        windownList_.emplace_back(ui);
    }
    void removeFromUiCore(MUiWindBase* ui)
    {
        std::lock_guard<std::mutex> lock(windownListMutex_);
        for(auto it = windownList_.begin(); it != windownList_.end(); it++)
        {
            if(*it == ui)
            {
                windownList_.erase(it);
                return;
            }
        }
    }
    void updateUiNotify(MUiBase* ui)
    {
        stUIEvent* event = new stUIEvent;
        if(!event || !ui)
        {
            return;
        }
        event->eventId = E_UI_EVNET_ID_UPDATE;
        event->data = reinterpret_cast<uint8_t*>(ui);
        event->dataLen = 4;
        event->clean = [event](){
            if(event)
            {
                delete event;
            }
        };
        msg_->sendQueue(event);
    }
    void updateUiNotify(MUiWindBase* ui)
    {
        stUIEvent* event = new stUIEvent;
        if(!event || !ui)
        {
            return;
        }
        event->eventId = E_UI_EVNET_ID_WINDOWN_UPDATE;
        event->data = reinterpret_cast<uint8_t*>(ui);
        event->dataLen = 4;
        event->clean = [event](){
            if(event)
            {
                delete event;
            }
        };
        msg_->sendQueue(event);
    }
    void updateUiNotify(MUIKeyID keyVal, bool blongPress, uint32_t timerNum, bool brelease)
    {
        stUIEvent* event = new stUIEvent;
        if(!event)
        {
            return;
        }
        stUIKeyEvent* key = new stUIKeyEvent;
        if(! key)
        {
            delete event;
            return;
        }
        key->keyVal = keyVal;
        key->blongPress = blongPress;
        key->timerNum = timerNum;
        key->brelease = brelease;
        event->eventId = E_UI_EVENT_ID_KEY_PRESSDOWN;
        event->data = reinterpret_cast<uint8_t*>(key);
        event->dataLen = 4;
        event->clean = [event,key](){
            if(event)
            {
                delete event;
            }
            if(key)
            {
                delete key;
            }
        };
        msg_->sendQueue(event);
    }
    uint32_t getPanelWidth()
    {
        if(lcd_)
        {
            return lcd_->getWidth();
        }
        else
        {
            return 0;
        }
    }
    uint32_t getPanelHeight()
    {
        if(lcd_)
        {
            return lcd_->getHeight();
        }
        else
        {
            return 0;
        }
    }
    uint32_t getRamSize() const
    {
        return lcdRamSize_;
    }
    uint32_t getMaxDataSendSize() const {return lcd_->getMaxDataSendSize();}
    void setFlushAddress(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
    {
        lcd_->setAddress(x1,y1,x2,y2);
    }
    void sendData(const uint8_t* data, size_t len)
    {
        lcd_->sendData(data, len);
    }
private:
    MUicore() : lcd_(nullptr), lcdRamSize_(0), msg_(new MsgQueue<stUIEvent*>),foucsedUi_(nullptr),showWindow_(nullptr)
    {
        static std::thread uiThread_([this](){
            bool brefreshBackGround = false;
            while(true)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                stUIEvent* event = msg_->getQueueData();
                if(event && event->eventId == E_UI_EVNET_ID_WINDOWN_UPDATE)
                {
                    MUiWindBase* wind = reinterpret_cast<MUiWindBase*>(event->data);
                    if(lcd_ && wind)
                    {
                        if(wind->bshow() && wind != showWindow_)
                        {
                            showWindow_ = wind;
                            wind->refreshFullWindown();
                            foucsedUi_ = showWindow_->getFocusUi();
                            if(foucsedUi_ && foucsedUi_->canBefocused() && !foucsedUi_->bfocused())
                            {
                                foucsedUi_->setFocused(true);
                            }
                            if(foucsedUi_)
                            {
                                foucsedUi_->updateData();
                                foucsedUi_->drawFocus();
                                foucsedUi_->onFocus();
                            }
                            showWindow_->flushFrame();
                        }
                    }
                }
                else if(event && event->eventId == E_UI_EVNET_ID_UPDATE)//UPDATE ui
                {
                    MUiBase* ui = reinterpret_cast<MUiBase*>(event->data);
                    if(lcd_ && ui)
                    {
                        brefreshBackGround = ui->needUpdateBackGround();
                        if(brefreshBackGround)
                        {
                            
                            ui->getWindow()->refreshFullWindown();
                            //printf("update BackGround\n");
                        }
                        else
                        {
                            //printf("update range (%u,%u,%u,%u)\n",0, ui->getY(), lcd_->getWidth(), ui->getHeight());
                            ui->getWindow()->refreshPartWindown(ui);
                        }
                        if(!showWindow_)
                        {
                            std::lock_guard<std::mutex> lock(windownListMutex_);
                            for(auto& it : windownList_)
                            {
                                if(it->bshow())
                                {
                                    showWindow_ = it;
                                    break;
                                }
                            }
                        }
                        if(!foucsedUi_ && showWindow_)
                        {
                            foucsedUi_ = showWindow_->getFocusUi();
                            if(foucsedUi_ && foucsedUi_->canBefocused())
                            {
                                foucsedUi_->setFocused(true);
                            }
                        }
                        /*if(ui->getWindow()->bshow() && ui->getWindow() != showWindow_)
                        {
                            showWindow_ = ui->getWindow();
                            foucsedUi_ = showWindow_->getFocusUi();
                            if(foucsedUi_ && foucsedUi_->canBefocused() && !ui->bfocused())
                            {
                                foucsedUi_->setFocused(true);
                            }
                        }*/
                        if(foucsedUi_)
                        {
                            foucsedUi_->updateData();
                            foucsedUi_->drawFocus();
                            foucsedUi_->onFocus();
                        }
                        if(brefreshBackGround)
                        {
                            if(showWindow_ == ui->getWindow())
                            showWindow_->flushFrame();
                        }
                        else
                        {
                            if(showWindow_ == ui->getWindow())
                            showWindow_->flusRange(ui->getY(),ui->getHeight());
                        }
                    }
                }
                else if(event && event->eventId == E_UI_EVENT_ID_KEY_PRESSDOWN)
                {
                    stUIKeyEvent* key = reinterpret_cast<stUIKeyEvent*>(event->data);
                    switch (key->keyVal)
                    {
                    case E_UI_KEY_EVNET_ID_OK:
                        if(foucsedUi_)
                        {
                            printf("key(ok)pressdown, blongPress = %d, timerNum = %lu, brelease = %d\n",key->blongPress, key->timerNum, key->brelease);
                            foucsedUi_->pressDown(E_UI_EVENT_ID_KEY_PRESSDOWN, key->keyVal, key->blongPress, key->timerNum, key->brelease);
                        }
                        if(!key->blongPress && key->brelease)
                        {
                            //switchFocus(E_UI_FOCUS_NEXT);
                        }
                        break;
                    case E_UI_KEY_EVNET_ID_UP:
                        if(!key->brelease)
                        {
                            printf("key(ok)pressdown, blongPress = %d, timerNum = %lu, brelease = %d\n",key->blongPress, key->timerNum, key->brelease);
                            showWindow_->switchFocus(E_UI_FOCUS_FRONT);
                            foucsedUi_ = showWindow_->getFocusUi();
                        }
                        break;
                    case E_UI_KEY_EVNET_ID_DOWN:
                        if(!key->brelease)
                        {
                            printf("key(ok)pressdown, blongPress = %d, timerNum = %lu, brelease = %d\n",key->blongPress, key->timerNum, key->brelease);
                            showWindow_->switchFocus(E_UI_FOCUS_NEXT);
                            foucsedUi_ = showWindow_->getFocusUi();
                        }
                        break;
                    case E_UI_KEY_EVNET_ID_LEFT:
                        break;
                    case E_UI_KEY_EVNET_ID_RIGHT:
                        break;
                    case E_UI_KEY_EVNET_ID_MAX:
                        /* code */
                        break;
                    default:
                        break;
                    }
                }
                if(event && event->clean)
                {
                    event->clean();
                }
            }
        });
        uiThread_.detach();
    }
    ~MUicore()
    {
        if(msg_)
        {
            delete msg_;
            msg_ = nullptr;
        }
        lcd_ = nullptr;
    }
private:
    MUiInterface* lcd_;
    uint32_t lcdRamSize_;
    MsgQueue<stUIEvent*>* msg_;
    MUiBase* foucsedUi_;
    MUiWindBase* showWindow_;
    std::mutex windownListMutex_;
    std::list<MUiWindBase*> windownList_;
};