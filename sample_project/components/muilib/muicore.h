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

enum FocusSwitch
{
    E_UI_FOCUS_NEXT,
    E_UI_FOCUS_FRONT,
    E_UI_FOCUS_LEFT,
    E_UI_FOCUS_RIGHT
};

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
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if(!lcdRam_ && !lcdRamBack_)
            {
                lcdRamSize_ = lcd_->getWidth()*lcd_->getHeight()*lcd_->getPixelBytes();
                lcdRam_ = new uint8_t[lcdRamSize_];
                lcdRamBack_ = new uint8_t[lcdRamSize_];
                if(lcdRam_)
                {
                    memset(lcdRam_, 0, lcdRamSize_);
                }
                if(lcdRamBack_)
                {
                    memset(lcdRamBack_, 0, lcdRamSize_);
                }
            }
        }//becareful of dead lock
        refreshFrame();
    }

    void fillRect( int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color)
    {
        // Clipping
        if ((x >= lcd_->getWidth()) || (y >= lcd_->getHeight())) return;

        if (x < 0) {
            w += x;
            x = 0;
        }
        if (y < 0) {
            h += y;
            y = 0;
        }

        if ((x + w) > lcd_->getWidth())  w = lcd_->getWidth()  - x;
        if ((y + h) > lcd_->getHeight()) h = lcd_->getHeight() - y;

        if ((w < 1) || (h < 1)) return;
        std::lock_guard<std::mutex> lock(mutex_);
        for(uint32_t j = 0; j < h; j++)
        {
            for(uint32_t i = 0; i < w; i++)
            {
                drawPixel(x + i, y + j, color);
            }
        }
    }

    void fillScreen( uint32_t color)
    {
        fillRect( 0, 0, lcd_->getWidth(), lcd_->getHeight(), color);
    }
    void drawPixel(int32_t x, int32_t y, uint32_t color)
    {
        uint8_t val;
        uint32_t index = (x + (y*lcd_->getWidth()))*2;
        val = color >> 8 ;
        lcdRam_[index] = val;

        val = color;
        lcdRam_[index + 1] = val;
    }

    void drawChar(uint16_t x, uint16_t y, uint8_t num, uint8_t mode, uint16_t color, uint16_t backColor)
    {
        uint8_t temp;
        uint8_t pos, t;
        if(color == backColor)
        {
            backColor = ~color;
        }
        if (x > lcd_->getWidth() - 16 || y > lcd_->getHeight() - 16)return;
        num = num - ' ';
        std::lock_guard<std::mutex> lock(mutex_);
        if (!mode) {
            for (pos = 0; pos < 16; pos++) {
                temp = asc2_1608[(uint16_t)num * 16 + pos];
                for (t = 0; t < 8; t++) {
                    if (temp & 0x01)
                        drawPixel(x + t, y + pos, color);
                    else
                        drawPixel(x + t, y + pos, backColor);
                    temp >>= 1;
                }
            }
        } else {
            for (pos = 0; pos < 16; pos++) {
                temp = asc2_1608[(uint16_t)num * 16 + pos];
                for (t = 0; t < 8; t++) {
                    if (temp & 0x01)
                        drawPixel(x + t, y + pos, color);
                    temp >>= 1;
                }
            }
        }
    }

    void drawString(uint16_t x, uint16_t y, const char *p, uint16_t color, uint16_t backColor, uint16_t* height)
    {
        *height = 16;
        while (*p != '\0') {
            if (x > lcd_->getWidth() - 16) {
                x = 0;
                y += 16;
                *height += 16;
            }
            if (y > lcd_->getHeight() - 16) {
                y = x = 0;
                fillScreen(TFT_RED);
            }
            drawChar(x, y, *p, 0, color, backColor);
            x += 8;
            p++;
        }
    }

    void drawString(uint16_t x, uint16_t y, const char *p, uint16_t color, uint16_t* height)
    {
        *height = 16;
        while (*p != '\0') {
            if (x > lcd_->getWidth() - 16) {
                x = 0;
                y += 16;
                *height += 16;
            }
            if (y > lcd_->getHeight() - 16) {
                //y = x = 0;
                //fillScreen(TFT_RED);
            }
            drawChar(x, y, *p, 1, color, color);
            x += 8;
            p++;
        }
    }
    void setRamData(const uint8_t* data, uint32_t len)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        memcpy(lcdRam_, data, len > lcdRamSize_ ? lcdRamSize_ : len);
    }
    void setRamData(int32_t x, int32_t y, int32_t w, int32_t h, const unsigned char* picture, uint32_t len)
    {
        if(!picture || len == 0)
        {
            return;
        }
        if ((x >= lcd_->getWidth()) || (y >= lcd_->getHeight())) return;

        if (x < 0) {
            w += x;
            x = 0;
        }
        if (y < 0) {
            h += y;
            y = 0;
        }

        if ((x + w) > lcd_->getWidth())  w = lcd_->getWidth()  - x;
        if ((y + h) > lcd_->getHeight()) h = lcd_->getHeight() - y;

        if ((w < 1) || (h < 1)) return;
        const unsigned char* p = picture;
        uint32_t pixelWidth = w*lcd_->getPixelBytes();
        uint32_t xmap = x*lcd_->getPixelBytes();
        uint32_t xlen = lcd_->getWidth()*lcd_->getPixelBytes();
        if( (x == 0) && (y == 0) && (w = lcd_->getWidth()) && (h == lcd_->getHeight()))
        {
            setRamData(picture, lcdRamSize_);
        }
        else
        {
            std::lock_guard<std::mutex> lock(mutex_);
            for(uint32_t j = 0; j < h; j++)
            {
                for(uint32_t i = 0; i < pixelWidth; i++)
                {
                    lcdRam_[(uint32_t)((xmap+i) + (y+j)*xlen)] = *(p++);
                    if(p >= picture + len)
                    {
                        return;
                    }
                }
            }
        }
    }
    void resetPartRam(int32_t x, int32_t y, int32_t w, int32_t h)
    {
        if ((x >= lcd_->getWidth()) || (y >= lcd_->getHeight())) return;

        if (x < 0) {
            w += x;
            x = 0;
        }
        if (y < 0) {
            h += y;
            y = 0;
        }

        if ((x + w) > lcd_->getWidth())  w = lcd_->getWidth()  - x;
        if ((y + h) > lcd_->getHeight()) h = lcd_->getHeight() - y;

        if ((w < 1) || (h < 1)) return;

        uint32_t pixelWidth = w*lcd_->getPixelBytes();
        uint32_t xmap = x*lcd_->getPixelBytes();
        uint32_t xlen = lcd_->getWidth()*lcd_->getPixelBytes();
        if( (x == 0) && (y == 0) && (w = lcd_->getWidth()) && (h == lcd_->getHeight()))
        {
            setRamData(lcdRamBack_, lcdRamSize_);
        }
        else
        {
            std::lock_guard<std::mutex> lock(mutex_);
            for(uint32_t j = 0; j < h; j++)
            {
                for(uint32_t i = 0; i < pixelWidth; i++)
                {
                    lcdRam_[(uint32_t)((xmap+i) + (y+j)*xlen)] = lcdRamBack_[(uint32_t)((xmap+i) + (y+j)*xlen)];
                }
            }
        }
    }
    void getRamData(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* data, uint32_t len)
    {
        if(!data || len == 0)
        {
            return;
        }
        if ((x >= lcd_->getWidth()) || (y >= lcd_->getHeight())) return;

        if (x < 0) {
            w += x;
            x = 0;
        }
        if (y < 0) {
            h += y;
            y = 0;
        }

        if ((x + w) > lcd_->getWidth())  w = lcd_->getWidth()  - x;
        if ((y + h) > lcd_->getHeight()) h = lcd_->getHeight() - y;

        if ((w < 1) || (h < 1)) return;

        uint32_t pixelWidth = w*lcd_->getPixelBytes();
        uint32_t xmap = x*lcd_->getPixelBytes();
        uint32_t xlen = lcd_->getWidth()*lcd_->getPixelBytes();
        if( (x == 0) && (y == 0) && (w = lcd_->getWidth()) && (h == lcd_->getHeight()))
        {
            memcpy(data, lcdRam_, len);
        }
        else
        {
            std::lock_guard<std::mutex> lock(mutex_);
            for(uint32_t j = 0; j < h; j++)
            {
                for(uint32_t i = 0; i < pixelWidth; i++)
                {
                    *(data++) = lcdRam_[(uint32_t)((xmap+i) + (y+j)*xlen)];
                    len --;
                    if(len == 0)
                    {
                        return;
                    }
                }
            }
        }
    }
    void drawLine(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color)
    {
        fillRect(x, y, w, h, color);
    }
    void drawFocus(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color)
    {
        drawLine(x, y, w, 2, color);
        drawLine(x, y, 2, h, color);
        drawLine(x, y+h-2, w, 2, color);
        drawLine(x+w-2, y, 2, h, color);
    }
    uint8_t getPixelBytes() const
    {
        if(lcd_)
        {
            return lcd_->getPixelBytes();
        }
        return 0;
    }
    void setBackGround(uint32_t color)
    {
        if(!lcd_)
        {
            return;
        }

        fillScreen(color);
        memcpy(lcdRamBack_, lcdRam_, lcdRamSize_);
        refreshFrame();
    }
    void setBackGround(const uint8_t* picture, uint32_t len)
    {
        if(!lcd_ || !picture || len != lcdRamSize_)
        {
            return;
        }
        setRamData(0,0,lcd_->getWidth(),lcd_->getHeight(),picture,len);
        memcpy(lcdRamBack_, lcdRam_, lcdRamSize_);
        refreshFrame();
    }
    void addToUiCore(MUiBase* ui)
    {
        std::lock_guard<std::mutex> lock(uiListMutex_);
        for(auto& it : uiList_)
        {
            if(it == ui)
            {
                return;
            }
        }
        uiList_.emplace_back(ui);
    }
    void removeFromUiCore(MUiBase* ui)
    {
        std::lock_guard<std::mutex> lock(uiListMutex_);
        for(auto it = uiList_.begin(); it != uiList_.end(); it++)
        {
            if(*it == ui)
            {
                uiList_.erase(it);
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
    void updateUiNotify(MUIKeyID keyVal, bool blongPress, bool bdoubleClick, uint32_t timerNum, bool brelease)
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
        key->bdoubleClick = bdoubleClick;
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
private:
    MUicore() : lcd_(nullptr), lcdRam_(nullptr), lcdRamBack_(nullptr), lcdRamSize_(0), msg_(new MsgQueue<stUIEvent*>),foucsedUi_(nullptr)
    {
        static std::thread uiThread_([this](){
            bool brefreshBackGround = false;
            while(true)
            {
                stUIEvent* event = msg_->getQueueData();
                if(event && event->eventId == E_UI_EVNET_ID_UPDATE)//UPDATE ui
                {
                    MUiBase* ui = reinterpret_cast<MUiBase*>(event->data);
                    if(lcd_ && lcdRam_ && lcdRamBack_ && ui)
                    {
                        brefreshBackGround = ui->needUpdateBackGround();
                        if(brefreshBackGround)
                        {
                            resetPartRam(0, 0, lcd_->getWidth(), lcd_->getHeight());
                            std::lock_guard<std::mutex> lock(uiListMutex_);
                            for(auto& it : uiList_)
                            {
                                if(it->bInited())
                                {
                                    if(it->bfocused() && it->canBefocused())
                                    {
                                        foucsedUi_ = it;
                                        continue;
                                    }
                                    it->updateData();
                                }
                            }
                            printf("update BackGround\n");
                        }
                        else
                        {
                            printf("update range (%u,%u,%u,%u)\n",0, ui->getY(), lcd_->getWidth(), ui->getHeight());
                            std::lock_guard<std::mutex> lock(uiListMutex_);
                            for(auto& it : uiList_)
                            {
                                if(it == ui)
                                {
                                    if(it->bInited())
                                    {
                                        resetPartRam(0, ui->getY(), lcd_->getWidth(), ui->getHeight());
                                        ui->updateData();
                                    }
                                }
                                if(it->bInited())
                                {
                                    if(it->bfocused() && it->canBefocused())
                                    {
                                        foucsedUi_ = it;
                                        continue;
                                    }
                                }
                            }
                        }
                        if(!foucsedUi_)
                        {
                            std::lock_guard<std::mutex> lock(uiListMutex_);
                            for(auto& it : uiList_)
                            {
                                if(it->canBefocused())
                                {
                                    foucsedUi_ = it;
                                    break;
                                }
                            }
                            if(foucsedUi_ && foucsedUi_->canBefocused())
                            {
                                foucsedUi_->setFocused(true);
                            }
                        }
                        if(foucsedUi_)
                        {
                            //setRamData(foucsedUi_->getX(),foucsedUi_->getY(),foucsedUi_->getWidth(),foucsedUi_->getHeight(),foucsedUi_->getUi(),foucsedUi_->getUiDataLen());
                            foucsedUi_->updateData();
                            drawFocus(foucsedUi_->getX(), foucsedUi_->getY(), foucsedUi_->getWidth(), foucsedUi_->getHeight(), TFT_RED);
                            foucsedUi_->onFocus();
                        }
                        if(brefreshBackGround)
                        {
                            refreshFrame();;
                        }
                        else
                        {
                            refreshRange(ui->getY(),ui->getHeight());
                        }
                    }
                }
                else if(event && event->eventId == E_UI_EVENT_ID_KEY_PRESSDOWN)
                {
                    stUIKeyEvent* key = reinterpret_cast<stUIKeyEvent*>(event->data);
                    switch (key->keyVal)
                    {
                    case E_UI_KEY_EVNET_ID_OK:
                        printf("key(ok)pressdown, blongPress = %d, timerNum = %lu, brelease = %d\n",key->blongPress, key->timerNum, key->brelease);
                        if(foucsedUi_)
                        {
                            foucsedUi_->pressDown(E_UI_EVENT_ID_KEY_PRESSDOWN, key->keyVal, key->blongPress, key->bdoubleClick, key->timerNum, key->brelease);
                        }
                        switchFocus(E_UI_FOCUS_NEXT);
                        break;
                    case E_UI_KEY_EVNET_ID_UP:
                        switchFocus(E_UI_FOCUS_FRONT);
                        break;
                    case E_UI_KEY_EVNET_ID_DOWN:
                        switchFocus(E_UI_FOCUS_NEXT);
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
        if(lcdRam_)
        {
            delete [] lcdRam_;
            lcdRam_ = nullptr;
        }
        if(lcdRamBack_)
        {
            delete [] lcdRamBack_;
            lcdRamBack_ = nullptr;
        }
        lcd_ = nullptr;
    }
    
    void refreshRange(uint16_t y, uint16_t height)
    {
        if(!lcdRam_)
        {
            return;
        }
        uint16_t x = 0;
        uint16_t width = lcd_->getWidth();
        uint32_t len = ((width*lcd_->getPixelBytes()) * height);
        uint8_t* data = new uint8_t[len];
        memset(data, 0, len);
        getRamData(x, y, width, height, data, len);
        lcd_->setAddress(x,y,width+x-1,height+y-1);
        std::lock_guard<std::mutex> lock(mutex_);
        if(len > lcd_->getMaxDataSendSize())
        {
            for(uint32_t i = 0; i < len; i += lcd_->getMaxDataSendSize())
            {
                if(len - i > lcd_->getMaxDataSendSize())
                {
                    lcd_->sendData(data + i, lcd_->getMaxDataSendSize());
                }
                else
                {
                    lcd_->sendData(data + i, len - i);
                }
            }
        }
        else
        {
            lcd_->sendData(data, len);
        }
        delete [] data;
    }
    void refreshFrame()
    {
        if(!lcdRam_)
        {
            return;
        }
        lcd_->setAddress(0,0,lcd_->getWidth()-1, lcd_->getHeight()-1);
        std::lock_guard<std::mutex> lock(mutex_);
        if(lcdRamSize_ > lcd_->getMaxDataSendSize())
        {
            for(uint32_t i = 0; i < lcdRamSize_; i += lcd_->getMaxDataSendSize())
            {
                if(lcdRamSize_ - i > lcd_->getMaxDataSendSize())
                {
                    lcd_->sendData(lcdRam_ + i, lcd_->getMaxDataSendSize());
                }
                else
                {
                    lcd_->sendData(lcdRam_ + i, lcdRamSize_ - i);
                }
            }
        }
        else
        {
            lcd_->sendData(lcdRam_, lcdRamSize_);
        }
    }
    void switchFocus(FocusSwitch eswitch)
    {
        bool skipNoFocusItem = false;
        switch (eswitch)
        {
            case E_UI_FOCUS_NEXT:
            {
                std::lock_guard<std::mutex> lock(uiListMutex_);
                for(auto it = uiList_.begin(); it != uiList_.end();)
                {
                    if(*(it++) == foucsedUi_ || skipNoFocusItem)
                    {
                        if(it == uiList_.end())
                        {
                            return;
                        }
                        skipNoFocusItem = true;
                        if(it != uiList_.end() && (*it)->canBefocused())
                        {
                            foucsedUi_->setFocused(false);
                            updateUiNotify(foucsedUi_);//old
                            foucsedUi_ = *it;
                            foucsedUi_ ->setFocused(true);
                            updateUiNotify(foucsedUi_);//new
                            return;
                        }
                    }
                }
                break;
            }
            case E_UI_FOCUS_FRONT:
            {
                std::lock_guard<std::mutex> lock(uiListMutex_);
                for(auto it = uiList_.rbegin(); it != uiList_.rend();)
                {
                    if(*(it++) == foucsedUi_ || skipNoFocusItem)
                    {
                        if(it == uiList_.rend())
                        {
                            return;
                        }
                        skipNoFocusItem = true;
                        if(it != uiList_.rend() && (*it)->canBefocused())
                        {
                            foucsedUi_->setFocused(false);
                            updateUiNotify(foucsedUi_);//old
                            foucsedUi_ = *it;
                            foucsedUi_ ->setFocused(true);
                            updateUiNotify(foucsedUi_);//new
                            return;
                        }
                    }
                }
                break;
            }
            case E_UI_FOCUS_LEFT:

                break;
            case E_UI_FOCUS_RIGHT:

                return;
            default:
                break;
        }
    }
private:
    MUiInterface* lcd_;
    uint8_t* lcdRam_;
    uint8_t* lcdRamBack_;
    uint32_t lcdRamSize_;
    MsgQueue<stUIEvent*>* msg_;
    MUiBase* foucsedUi_;
    std::mutex mutex_;
    std::mutex uiListMutex_;
    std::list<MUiBase*> uiList_;
};