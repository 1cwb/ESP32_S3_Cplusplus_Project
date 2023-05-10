#pragma once
#include "mmcpwm.h"
class Motor
{
public:
    Motor(int groupId, uint32_t pwmFreqHz, uint32_t resolutionHz, gpio_num_t gpioNumA, gpio_num_t gpioNumB):bEnable_(false),operator_(&timer_), comparatorA_(&operator_), comparatorB_(&operator_), generatorA_(&operator_), generatorB_(&operator_)
    {
        timer_.init(groupId, pwmFreqHz, resolutionHz);
        operator_.init();
        comparatorA_.init(true);
        comparatorB_.init(true);
        generatorA_.init(gpioNumA);
        generatorB_.init(gpioNumB);
        comparatorA_.setCompareVal(0);
        comparatorA_.setCompareVal(0);
        mcpwm_gen_timer_event_action_t timerEventActionEnd = {.direction = MCPWM_TIMER_DIRECTION_UP, .event = MCPWM_TIMER_EVENT_INVALID, .action = MCPWM_GEN_ACTION_HIGH};
        mcpwm_gen_compare_event_action_t compareEventActionEnd = {.direction = MCPWM_TIMER_DIRECTION_UP, .comparator = nullptr, .action = MCPWM_GEN_ACTION_LOW};
        generatorA_.setActionsOnTimerEvent(MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH),timerEventActionEnd);
        generatorA_.setActionsOnCompareEvent(MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparatorA_.getHandle(), MCPWM_GEN_ACTION_LOW),compareEventActionEnd);
        generatorB_.setActionsOnTimerEvent(MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH),timerEventActionEnd);
        generatorB_.setActionsOnCompareEvent(MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparatorB_.getHandle(), MCPWM_GEN_ACTION_LOW),compareEventActionEnd);
    }
    ~Motor()
    {
        if(bEnable_)
        {
            disable();
        }
        generatorA_.deinit();
        generatorB_.deinit();
        comparatorA_.deinit();
        comparatorB_.deinit();
        operator_.deinit();
        timer_.deinit();
    }
    Motor(const Motor&) = delete;
    Motor(Motor&&) = delete;
    Motor& operator=(const Motor&) =delete;
    Motor& operator=(Motor&&) =delete;
    bool enable()
    {
        if(!timer_.enable())
        {
            return false;
        }
        if(!timer_.startOrStop(MCPWM_TIMER_START_NO_STOP))
        {
            return false;
        }
        bEnable_ = true;
        return true;
    }
    bool disable()
    {
        if(!timer_.startOrStop(MCPWM_TIMER_STOP_EMPTY))
        {
            return false;
        }
        if(!timer_.disable())
        {
            return false;
        }
        bEnable_ = false;
        return true;
    }
    bool isEnable() const 
    {
        return bEnable_;
    }
    bool setSpeed(uint32_t speed)
    {
        if(!comparatorA_.setCompareVal(speed))
        {
            return false;
        }
        if(!comparatorB_.setCompareVal(speed))
        {
            return false;
        }
        return true;
    }
    bool forward()
    {
        if(!generatorA_.setForceLevel(-1, true))
        {
            return false;
        }
        if(!generatorB_.setForceLevel(0, true))
        {
            return false;
        }
        return true;
    }
    bool reverse()
    {
        if(!generatorA_.setForceLevel(0, true))
        {
            return false;
        }
        if(!generatorB_.setForceLevel(-1, true))
        {
            return false;
        }
        return true;
    }
    bool coast()
    {
        if(!generatorA_.setForceLevel(0, true))
        {
            return false;
        }
        if(!generatorB_.setForceLevel(0, true))
        {
            return false;
        }
        return true;
    }
    bool brake()
    {
        if(!generatorA_.setForceLevel(1, true))
        {
            return false;
        }
        if(!generatorB_.setForceLevel(1, true))
        {
            return false;
        }
        return true;
    }
private:
    bool bEnable_;
    McpwmTimer timer_;
    McpwmOperator operator_;
    McpwmComparator comparatorA_;
    McpwmComparator comparatorB_;
    McpwmGenerator generatorA_;
    McpwmGenerator generatorB_;
};