#pragma once
#include <iostream>
#include "mbutton.h"
#include "madc.h"
class MRocker
{
    using EventButtonPressCb = std::function<void(uint32_t, uint32_t, uint32_t, bool, uint32_t, bool)>;
public:
    MRocker(gpio_num_t gpioy, gpio_num_t gpiox, gpio_num_t btNum, adc_unit_t unitID = ADC_UNIT_1)
    {
        adcy = new MAdc;
        adcx = new MAdc;
        bt = new MButton(btNum);
        adcy->oneShortUnitInit(unitID, ADC_ULP_MODE_DISABLE);
        adcx->oneShortUnitInit(unitID, ADC_ULP_MODE_DISABLE);
        adcy->oneShortChanConfig(gpioy, ADC_ATTEN_DB_11, ADC_BITWIDTH_DEFAULT);
        adcy->caliFittingConfig();
        adcx->oneShortChanConfig(gpiox, ADC_ATTEN_DB_11, ADC_BITWIDTH_DEFAULT);
        adcx->caliFittingConfig();
    }
    ~MRocker()
    {
        adcy->caliFittingDestory();
        adcx->caliFittingDestory();
        adcy->oneShortUnitDeInit();
        adcx->oneShortUnitDeInit();
        if(adcy)
        {
            delete adcy;
        }
        if(adcx)
        {
            delete adcx;
        }
        if(bt)
        {
            delete bt;
        }
    }
    int getYvalue()
    {
        int v;
        if(adcy->calibrated())
        {
            adcy->caliRawToVoltage(&v);
        }
        else
        {
            v = -1;
        }
        return v;
    }
    int getXvalue()
    {
        int v;
        if(adcx->calibrated())
        {
            adcx->caliRawToVoltage(&v);
        }
        else
        {
            v = -1;
        }
        return v;
    }
    void setButtonPressCb(const EventButtonPressCb&& cb)
    {
        bt->setButtonPressCb(std::move(cb));
    }
private:
    MAdc* adcy;
    MAdc* adcx;
    MButton* bt;// = new MButton(GPIO_NUM_0);
};