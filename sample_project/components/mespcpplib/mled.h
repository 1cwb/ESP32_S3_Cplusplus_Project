#pragma once
#include "mgpio.h"
class MLed
{
public:
    MLed(int32_t pin);
    ~MLed();
    MLed(const MLed&) = delete;
    MLed(MLed&&) = delete;
    MLed& operator=(const MLed& ) = delete;
    MLed& operator=(MLed&& ) = delete;
    void ON();
    void OFF();
    int32_t getPinNum() const
    {
        return pin_;
    }
    bool isOn() const
    {
        return on_;
    }
private:
    int32_t pin_;
    bool on_;
    MGpio gpio_;
};