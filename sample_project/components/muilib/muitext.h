#pragma once
#include "muicore.h"
#include "muicommon.h"
#include "muiwindown.h"

class MUiText : public MUiBase
{
public:
    MUiText(MUIWindown* windown, uint16_t x, uint16_t y, bool autoRegisterIncore = false, bool canbefocus = false)
    :MUiBase(x,y,0,0,autoRegisterIncore,canbefocus),tempHeight_(0),dataLen_(0),data_(nullptr),color_(TFT_RED), backcolor_(0), windown_(windown)
    {
        memset(data_, 0, dataLen_);
        if(autoRegisterIncore_)
        {
            windown_->addSubUi(this);
        }
    }
    ~MUiText()
    {
        if(autoRegisterIncore_)
        {
            windown_->removeSubUi(this);
        }
        if(data_)
        {
            delete data_;
            data_ = nullptr;
            dataLen_ = 0;
        }
    }
    /*void setBackGround(uint16_t color)
    {
        uint16_t colorTemp = (color >> 8) | (color << 8);
        uint32_t len = dataLen_/MUicore::getInstance()->getPixelBytes();
        uint16_t* dataTemp = reinterpret_cast<uint16_t*> (data_);
        for(uint32_t i = 0; i < len; i++)
        {
            dataTemp[i] = colorTemp;
        }
        binited_ = true;
        MUicore::getInstance()->updateUiNotify(this);
        //MUicore::getInstance()->drawPicture(x_,y_,width_,height_,data_,dataLen_);
        //MUicore::getInstance()->drawPicture(20, 20, 32, 32, wifiIcon, sizeof(wifiIcon));
    }*/
    /*virtual const uint8_t* getUi() const
    {
        return data_;
    }
    virtual uint32_t getUiDataLen() const 
    {
        return dataLen_;
    }*/
    virtual void updateData()
    {
        if(!data_)
        {
            return;
        }
        if(backcolor_ != 0)
        {
            windown_->drawString(x_, y_, reinterpret_cast<const char*>(data_), color_, backcolor_, &height_);
        }
        else
        {
            windown_->drawString(x_, y_, reinterpret_cast<const char*>(data_), color_, &height_);
        }
        if(tempHeight_ <= height_)
        {
            tempHeight_ = height_;
        }
        else
        {
            uint32_t temp = height_;
            height_ = tempHeight_;
            tempHeight_ = temp;
        }
    }
    virtual void onFocus()
    {

    }
    virtual void drawFocus()
    {
        if(binited_ && bCanfocus_ && bfocused_)
        {
            //printf("draw focus x = %u, y = %u, width = %lu height = %u\n",x_,y_,MUicore::getInstance()->getPanelWidth(),height_);
            windown_->drawLine(0, y_, MUicore::getInstance()->getPanelWidth(), 2, focusColor_);
            windown_->drawLine(0, y_ + height_ - 2, MUicore::getInstance()->getPanelWidth(), 2, focusColor_);
        }
    }
    virtual MUiWindBase* getWindow()
    {
        return windown_;
    }
    void setText(const char* text, uint32_t userlen, uint16_t color, uint16_t backcolor = TFT_BLACK)
    {
        if(data_ && dataLen_ != 0)
        {
            delete data_;
            data_ = nullptr;
            dataLen_ = 0;
        }
        dataLen_ = strlen(text) + 1;
        data_ = new uint8_t[dataLen_];
        memset(data_,0,dataLen_);
        memcpy(data_,text,MIN(strlen(text),userlen));
        color_ = color;
        backcolor_ = backcolor;
        binited_ = true;
        if(autoRegisterIncore_)
        {
            windown_->updateUiNotify(this);
        }
    }
    uint16_t getBackColor() const 
    {
        return backcolor_;
    }
    void setIntNum(int32_t num, uint16_t userlen, uint16_t color, uint16_t backcolor = TFT_BLACK)
    {
        int len = 12;
        char buff[len];
        memset(buff, 0, len);
        snprintf(buff, len ,num < 0 ? "-%ld" : "%ld",num);
        setText(buff, userlen, color, backcolor);
    }
    const char* getStr() const
    {
        return reinterpret_cast<const char*> (data_);
    }
    void setXY(uint16_t x, uint16_t y)
    {
        if(x_ == x && y_ == y)
        {
            return;
        }
        x_ = x;
        y_ = y;
        if(autoRegisterIncore_)
        {
            bupgradeBack_ = true;
            windown_->updateUiNotify(this);
        }
    }
private:
    uint32_t tempHeight_;
    uint32_t dataLen_;
    uint8_t* data_;
    uint16_t color_;
    uint16_t backcolor_;
    MUIWindown* windown_;
};