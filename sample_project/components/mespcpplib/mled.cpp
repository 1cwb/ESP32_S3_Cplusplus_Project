#include "mled.h"

MLed::MLed(int32_t pin) 
: pin_(pin),
  on_(false),
  gpio_(static_cast<gpio_num_t>(pin), GPIO_MODE_OUTPUT, GPIO_PULLUP_DISABLE, GPIO_PULLDOWN_DISABLE, GPIO_INTR_DISABLE)
{

}
MLed::~MLed()
{

}
void MLed::ON()
{
    on_ = true;
    gpio_.setLevel(1);
}
void MLed::OFF()
{
    on_ = false;
    gpio_.setLevel(0);
}