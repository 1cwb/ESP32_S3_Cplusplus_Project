#pragma once
#include "driver/gpio.h"

class MGpio
{
public:
    MGpio(gpio_num_t pin, gpio_mode_t mode = GPIO_MODE_OUTPUT, gpio_pullup_t pullUpEn = GPIO_PULLUP_DISABLE, gpio_pulldown_t pullDownEn = GPIO_PULLDOWN_DISABLE, gpio_int_type_t intrType = GPIO_INTR_DISABLE);
    ~MGpio();
    MGpio(const MGpio&) = delete;
    MGpio(MGpio&&) = delete;
    MGpio& operator=(const MGpio& ) = delete;
    MGpio& operator=(MGpio&& ) = delete;
    bool setIntrType(gpio_int_type_t intrType = GPIO_INTR_NEGEDGE);
    bool intrEnable();
    bool intrDisable();
    int32_t getPin() const {return (int32_t) pin_;}
    bool installIsrService();
    void uninstallIsrService();
    bool addIsrHandler(gpio_isr_t isr_handler, void *args);
    bool removeIsrHandler();
    int32_t getLevel();
    bool setLevel(uint32_t level);
    bool setDirection(gpio_mode_t mode);
    bool setPullMode(gpio_pull_mode_t pullMode);
    bool wakeupEnable(gpio_int_type_t intrType);
    bool wakeupDiable();
    bool pullUpEnable();
    bool pullUpDiable();
    bool pullDownEnable();
    bool pullDownDiable();
private:
    gpio_num_t pin_;
    gpio_config_t gpioCfg_;
};