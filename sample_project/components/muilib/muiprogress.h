#pragma once
#include "muicore.h"
#include "muicommon.h"
#include "muiItem.h"
#include "muiwindown.h"

class MUIProgress
{
public:
    MUIProgress(MUIWindown* window, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t minLimit = 0, uint32_t maxLimit = 100)
    : progeress_(new MUiItem(window, x,y,width,height,true,false)),percentVal_(0),minLimit_(minLimit),maxLimit_(maxLimit),progressColor_(TFT_BLUE),bhor_(width > height ? true : false)
    {
        resetProgress();
    }
    ~MUIProgress()
    {
        if(progeress_)
        {
            delete progeress_;
            progeress_ = nullptr;
        }
    }
    void setBackColor(uint16_t backColor)
    {
        progeress_->setBackGround(backColor);
    }
    void setBarColor(uint16_t color)
    {
        progressColor_ = color;
    }
    void setVal(uint32_t val)
    {
        if(val < minLimit_ || val > maxLimit_ || maxLimit_ - minLimit_ <= 0)
        {
            return;
        }
        percentVal_ = 100*val/(maxLimit_ - minLimit_);
        char buff[6] = {0};
        snprintf(buff,6,"%ld%%",percentVal_);
        progeress_->setText(buff,TFT_RED);
        progeress_->setProgress(progressColor_, percentVal_, bhor_);
    }
    void resetProgress()
    {
        progeress_->setBackGround(progeress_->getBackGround());
        setVal(0);
    }
private:
    MUiItem* progeress_;
    uint32_t  percentVal_;
    uint32_t  minLimit_;
    uint32_t  maxLimit_;
    uint16_t progressColor_;
    bool     bhor_;
};