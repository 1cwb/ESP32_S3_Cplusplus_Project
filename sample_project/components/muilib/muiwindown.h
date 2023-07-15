#pragma once
#include "muicommon.h"
#include "muicore.h"


class MUIWindown : public MUiWindBase
{
public:
    MUIWindown() : foucsedUi_(nullptr), bshow_(false)
    {
        lcdRamSize_ = MUicore::getInstance()->getRamSize();
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
        MUicore::getInstance()->addToUiCore(this);
    }
    virtual ~MUIWindown()
    {
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
        MUicore::getInstance()->removeFromUiCore(this);
    }
    virtual bool bshow() const
    {
        return bshow_;
    }
    virtual uint32_t getDramSize() const
    {
        return lcdRamSize_;
    }
    virtual uint8_t* getDram()
    {
        return lcdRam_;
    }
    virtual uint8_t* getDramBack()
    {
        return lcdRamBack_;
    }
    MUIWindown(const MUIWindown&) = delete;
    MUIWindown(MUIWindown&&) = delete;
    MUIWindown& operator=(const MUIWindown& ) = delete;
    MUIWindown& operator=(MUIWindown&& ) = delete;
    void drawPixel(int32_t x, int32_t y, uint32_t color)
    {
        uint8_t val;
        uint32_t index = (x + (y*MUicore::getInstance()->getPanelWidth()))*2;
        val = color >> 8 ;
        lcdRam_[index] = val;

        val = color;
        lcdRam_[index + 1] = val;
    }
    void fillRect( int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color)
    {
        // Clipping
        if ((x >= MUicore::getInstance()->getPanelWidth()) || (y >= MUicore::getInstance()->getPanelHeight())) return;

        if (x < 0) {
            w += x;
            x = 0;
        }
        if (y < 0) {
            h += y;
            y = 0;
        }

        if ((x + w) > MUicore::getInstance()->getPanelWidth())  w = MUicore::getInstance()->getPanelWidth()  - x;
        if ((y + h) > MUicore::getInstance()->getPanelHeight()) h = MUicore::getInstance()->getPanelHeight() - y;

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
        fillRect( 0, 0, MUicore::getInstance()->getPanelWidth(), MUicore::getInstance()->getPanelHeight(), color);
    }
    void drawChar(uint16_t x, uint16_t y, uint8_t num, uint8_t mode, uint16_t color, uint16_t backColor)
    {
        uint8_t temp;
        uint8_t pos, t;
        if(color == backColor)
        {
            backColor = ~color;
        }
        if (x > MUicore::getInstance()->getPanelWidth() - 16 || y > MUicore::getInstance()->getPanelHeight() - 16)return;
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
            if (x > MUicore::getInstance()->getPanelWidth() - 16) {
                x = 0;
                y += 16;
                *height += 16;
            }
            if (y > MUicore::getInstance()->getPanelHeight() - 16) {
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
            if (x > MUicore::getInstance()->getPanelWidth() - 16) {
                x = 0;
                y += 16;
                *height += 16;
            }
            if (y > MUicore::getInstance()->getPanelHeight() - 16) {
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
        if ((x >= MUicore::getInstance()->getPanelWidth()) || (y >= MUicore::getInstance()->getPanelHeight())) return;

        if (x < 0) {
            w += x;
            x = 0;
        }
        if (y < 0) {
            h += y;
            y = 0;
        }

        if ((x + w) > MUicore::getInstance()->getPanelWidth())  w = MUicore::getInstance()->getPanelWidth()  - x;
        if ((y + h) > MUicore::getInstance()->getPanelHeight()) h = MUicore::getInstance()->getPanelHeight() - y;

        if ((w < 1) || (h < 1)) return;
        const unsigned char* p = picture;
        uint32_t pixelWidth = w*MUicore::getInstance()->getPixelBytes();
        uint32_t xmap = x*MUicore::getInstance()->getPixelBytes();
        uint32_t xlen = MUicore::getInstance()->getPanelWidth()*MUicore::getInstance()->getPixelBytes();
        if( (x == 0) && (y == 0) && (w = MUicore::getInstance()->getPanelWidth()) && (h == MUicore::getInstance()->getPanelHeight()))
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
    void getRamData(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t* data, uint32_t len)
    {
        if(!data || len == 0)
        {
            return;
        }
        if ((x >= MUicore::getInstance()->getPanelWidth()) || (y >= MUicore::getInstance()->getPanelHeight())) return;

        if (x < 0) {
            w += x;
            x = 0;
        }
        if (y < 0) {
            h += y;
            y = 0;
        }

        if ((x + w) > MUicore::getInstance()->getPanelWidth())  w = MUicore::getInstance()->getPanelWidth()  - x;
        if ((y + h) > MUicore::getInstance()->getPanelHeight()) h = MUicore::getInstance()->getPanelHeight() - y;

        if ((w < 1) || (h < 1)) return;

        uint32_t pixelWidth = w*MUicore::getInstance()->getPixelBytes();
        uint32_t xmap = x*MUicore::getInstance()->getPixelBytes();
        uint32_t xlen = MUicore::getInstance()->getPanelWidth()*MUicore::getInstance()->getPixelBytes();
        if( (x == 0) && (y == 0) && (w = MUicore::getInstance()->getPanelWidth()) && (h == MUicore::getInstance()->getPanelHeight()))
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
        return MUicore::getInstance()->getPixelBytes();
    }
    void setBackGround(uint32_t color)
    {
        fillScreen(color);
        memcpy(lcdRamBack_, lcdRam_, lcdRamSize_);
    }
    void setBackGround(const uint8_t* picture, uint32_t len)
    {
        if(!picture || len != lcdRamSize_)
        {
            return;
        }
        setRamData(0,0,MUicore::getInstance()->getPanelWidth(),MUicore::getInstance()->getPanelHeight(),picture,len);
        memcpy(lcdRamBack_, lcdRam_, lcdRamSize_);
    }
    virtual void resetPartRam(int32_t x, int32_t y, int32_t w, int32_t h)
    {
        if ((x >= getWindownWidth()) || (y >= getWindownHeight())) return;

        if (x < 0) {
            w += x;
            x = 0;
        }
        if (y < 0) {
            h += y;
            y = 0;
        }

        if ((x + w) > getWindownWidth())  w = getWindownWidth() - x;
        if ((y + h) > getWindownHeight()) h = getWindownHeight() - y;

        if ((w < 1) || (h < 1)) return;

        uint32_t pixelWidth = w*getPixelBytes();
        uint32_t xmap = x*getPixelBytes();
        uint32_t xlen = getWindownWidth()*getPixelBytes();
        if( (x == 0) && (y == 0) && (w = getWindownWidth()) && (h == getWindownHeight()))
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
    void addSubUi(MUiBase* ui)
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
    void removeSubUi(MUiBase* ui)
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
        MUicore::getInstance()->updateUiNotify(ui);
    }
    virtual void switchFocus(FocusSwitch eswitch)
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
                            MUicore::getInstance()->updateUiNotify(foucsedUi_);//old
                            foucsedUi_ = *it;
                            foucsedUi_ ->setFocused(true);
                            MUicore::getInstance()->updateUiNotify(foucsedUi_);//new
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
                            MUicore::getInstance()->updateUiNotify(foucsedUi_);//old
                            foucsedUi_ = *it;
                            foucsedUi_ ->setFocused(true);
                            MUicore::getInstance()->updateUiNotify(foucsedUi_);//new
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
    void show(bool bshow)
    {
        bshow_ = bshow;
        MUicore::getInstance()->updateUiNotify(this);
    }
    virtual uint32_t getWindownWidth() const 
    {
        return MUicore::getInstance()->getPanelWidth();
    }
    virtual uint32_t getWindownHeight() const 
    {
        return MUicore::getInstance()->getPanelHeight();
    }
    virtual void refreshFullWindown()
    {
        resetPartRam(0, 0, getWindownWidth(), getWindownHeight());
        std::lock_guard<std::mutex> lock(uiListMutex_);
        for(auto& it : uiList_)
        {
            if(it->bInited())
            {
                if(it->bfocused() && it->canBefocused())
                {
                    foucsedUi_ = it;
                }
                it->updateData();
            }
        }
    }
    virtual void refreshPartWindown(MUiBase* ui)
    {
        std::lock_guard<std::mutex> lock(uiListMutex_);
        for(auto& it : uiList_)
        {
            if(it == ui)
            {
                if(it->bInited())
                {
                    resetPartRam(0, ui->getY(), getWindownWidth(), getWindownHeight());
                    ui->updateData();
                }
            }
            if(it->bInited())
            {
                if(it->bfocused() && it->canBefocused())
                {
                    foucsedUi_ = it;
                }
            }
        }
    }
    virtual MUiBase* getFocusUi()
    {
        {
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
            }
        }
        return foucsedUi_;
    }
    virtual void flushFrame()
    {
        if(!lcdRam_)
        {
            return;
        }
        MUicore::getInstance()->setFlushAddress(0,0,getWindownWidth() -1, getWindownHeight()-1);
        std::lock_guard<std::mutex> lock(mutex_);
        if(lcdRamSize_ > MUicore::getInstance()->getMaxDataSendSize())
        {
            for(uint32_t i = 0; i < lcdRamSize_; i += MUicore::getInstance()->getMaxDataSendSize())
            {
                if(lcdRamSize_ - i > MUicore::getInstance()->getMaxDataSendSize())
                {
                    MUicore::getInstance()->sendData(lcdRam_ + i, MUicore::getInstance()->getMaxDataSendSize());
                }
                else
                {
                    MUicore::getInstance()->sendData(lcdRam_ + i, lcdRamSize_ - i);
                }
            }
        }
        else
        {
            MUicore::getInstance()->sendData(lcdRam_, lcdRamSize_);
        }
    }
    virtual void flusRange(uint16_t y, uint16_t height)
    {
        if(!lcdRam_)
        {
            return;
        }
        uint16_t x = 0;
        uint16_t width = getWindownWidth();
        uint32_t len = ((width*getPixelBytes()) * height);
        uint8_t* data = new uint8_t[len];
        memset(data, 0, len);
        getRamData(x, y, width, height, data, len);
        MUicore::getInstance()->setFlushAddress(x,y,width+x-1,height+y-1);
        std::lock_guard<std::mutex> lock(mutex_);
        if(len > MUicore::getInstance()->getMaxDataSendSize())
        {
            for(uint32_t i = 0; i < len; i += MUicore::getInstance()->getMaxDataSendSize())
            {
                if(len - i > MUicore::getInstance()->getMaxDataSendSize())
                {
                    MUicore::getInstance()->sendData(data + i, MUicore::getInstance()->getMaxDataSendSize());
                }
                else
                {
                    MUicore::getInstance()->sendData(data + i, len - i);
                }
            }
        }
        else
        {
            MUicore::getInstance()->sendData(data, len);
        }
        delete [] data;
    }
private:
    uint8_t* lcdRam_;
    uint8_t* lcdRamBack_;
    uint32_t lcdRamSize_;
    MUiBase* foucsedUi_;
    std::mutex mutex_;
    std::mutex uiListMutex_;
    std::list<MUiBase*> uiList_;
    bool bshow_;
};