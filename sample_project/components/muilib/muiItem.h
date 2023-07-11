#pragma once

#include "muicore.h"
#include "muicommon.h"
#include "muitext.h"

#define GET_TEXT_MIDDLE_X(xstatr, itemWidth, str) ((xstatr)+2 + ((((itemWidth) - 4) -  MIN((strlen(str)*8), (itemWidth) - 4)) >> 1))
#define GET_TEXT_MIDDLE_Y(ystart, itemHeight)     ((ystart) + (((itemHeight)>>1) - 8))
class MUiItem : public MUiBase
{
public:
    MUiItem(uint16_t x, uint16_t y, uint16_t width, uint16_t height, bool autoRegisterIncore = true, bool canbefocus = true)
    :MUiBase(x,y,width,height,autoRegisterIncore,canbefocus),backColor_(TFT_BLACK),backPic_(nullptr),
    dataLen_(width*height*MUicore::getInstance()->getPixelBytes()),picLen_(0),data_(new uint8_t[dataLen_]),
    ptext_(new MUiText(x,y))
    {
        setType(E_UI_TYPE_ITEM);
        memset(data_, 0, dataLen_);
        if(autoRegisterIncore_)
        {
            MUicore::getInstance()->addToUiCore(this);
        }
    }
    ~MUiItem()
    {
        if(autoRegisterIncore_)
        {
            MUicore::getInstance()->removeFromUiCore(this);
        }
        if(data_)
        {
            delete []data_;
            data_ = nullptr;
        }
        if(ptext_)
        {
            delete ptext_;
            ptext_ = nullptr;
        }
    }
    void setBackGround(uint16_t color)
    {
        uint16_t colorTemp = (color >> 8) | (color << 8);
        uint32_t len = dataLen_/MUicore::getInstance()->getPixelBytes();
        uint16_t* dataTemp = reinterpret_cast<uint16_t*> (data_);
        for(uint32_t i = 0; i < len; i++)
        {
            dataTemp[i] = colorTemp;
        }
        backColor_ = color;
        binited_ = true;
        if(autoRegisterIncore_)
        {
            MUicore::getInstance()->updateUiNotify(this);
        }
    }
    void setBackGround(const uint8_t* picture, uint32_t len)
    {
        /*uint16_t colorTemp = (color >> 8) | (color << 8);
        uint32_t len = dataLen_/MUicore::getInstance()->getPixelBytes();
        uint16_t* dataTemp = reinterpret_cast<uint16_t*> (data_);
        for(uint32_t i = 0; i < len; i++)
        {
            dataTemp[i] = colorTemp;
        }
        backColor_ = color;*/
        backPic_ = picture;
        picLen_ = len;
        binited_ = true;
        memcpy(data_, picture ,dataLen_);
        if(autoRegisterIncore_)
        {
            MUicore::getInstance()->updateUiNotify(this);
        }
        //MUicore::getInstance()->drawPicture(x_,y_,width_,height_,data_,dataLen_);
        //MUicore::getInstance()->drawPicture(20, 20, 32, 32, wifiIcon, sizeof(wifiIcon));
    }
    uint16_t getBackGround() const
    {
        return backColor_;
    }
    virtual const uint8_t* getUi() const
    {
        return data_;
    }
    virtual uint32_t getUiDataLen() const 
    {
        return dataLen_;
    }
    virtual void onFocus()
    {

    }
    virtual void drawFocus()
    {
        if(binited_ && bCanfocus_ && bfocused_)
        {
            //MUicore::getInstance()->drawLine(x_, y_, width_, 2, focusColor_);
            //MUicore::getInstance()->drawLine(x_, y_, 2, height_, focusColor_);
            //MUicore::getInstance()->drawLine(x_, y_+height_-2, width_, 2, focusColor_);
            //MUicore::getInstance()->drawLine(x_+ height_-2, y_, 2, height_, focusColor_);
            MUicore::getInstance()->drawFocus(x_, y_, width_, height_, focusColor_);
        }
    }
    void setText(const char* text, uint16_t color, uint16_t backcolor = TFT_BLACK)
    {
        if(ptext_)
        {
            ptext_->setXY(GET_TEXT_MIDDLE_X(x_, width_, text), GET_TEXT_MIDDLE_Y(y_,height_));
            ptext_->setText(text, (width_ - 4)>>3, color, backcolor);
            if(autoRegisterIncore_)
            {
                MUicore::getInstance()->updateUiNotify(this);
            }
        }
    }
    void setIntNum(int32_t num, uint16_t color, uint16_t backcolor = TFT_BLACK)
    {
        if(ptext_)
        {
            int len = 12;
            char buff[len];
            memset(buff, 0, len);
            snprintf(buff, len ,num < 0 ? "-%ld" : "%ld",num);
            setText(buff, color, backcolor);
            if(autoRegisterIncore_)
            {
                MUicore::getInstance()->updateUiNotify(this);
            }
        }
    }
    virtual void updateData()
    {
        MUicore::getInstance()->setRamData(x_, y_, width_, height_, data_, dataLen_);
        if(ptext_)
        {
            ptext_->updateData();
        }
    }
    void setXY(uint16_t x, uint16_t y)
    {
        if(x_ == x && y_ == y)
        {
            return;
        }
        x_ = x;
        y_ = y;
        if(ptext_)
        {
            ptext_->setXY(GET_TEXT_MIDDLE_X(x_, width_, ptext_->getStr()), GET_TEXT_MIDDLE_Y(y_,height_));
        }
        if(autoRegisterIncore_)
        {
            bupgradeBack_ = true;
            MUicore::getInstance()->updateUiNotify(this);
        }

    }
private:
    uint16_t backColor_;
    const uint8_t* backPic_;
    uint32_t dataLen_;
    uint32_t picLen_;
    uint8_t* data_;
    MUiText* ptext_;
};