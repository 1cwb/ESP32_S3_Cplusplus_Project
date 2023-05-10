#pragma once

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/mcpwm_prelude.h"
#include "stdarg.h"

class McpwmTimer
{
public:
    McpwmTimer():groupId_(0)
    {

    }
    ~McpwmTimer()
    {

    }
    McpwmTimer(const McpwmTimer&) = delete;
    McpwmTimer(McpwmTimer&&) = delete;
    McpwmTimer& operator=(const McpwmTimer&) =delete;
    McpwmTimer& operator=(McpwmTimer&&) =delete;
    mcpwm_timer_handle_t getHandle() const
    {
        return timerHandle_;
    }
    int getGroupId() const
    {
        return groupId_;
    }
    bool init(int groupId, uint32_t pwmFreqHz, uint32_t resolutionHz, mcpwm_timer_count_mode_t countMode = MCPWM_TIMER_COUNT_MODE_UP, bool updatePeriodOnEmpty = false, bool updatePeriodOnSync = false)
    {
        groupId_ = groupId;
        timerConfig_.group_id = groupId;
        timerConfig_.clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT;
        //分辨率为最小刻度单位，例如resolutionHz = 10MHZ， 一个tick就是0.1us
        timerConfig_.resolution_hz = resolutionHz;
        //周期是1秒时间内每-次变化或动作所消耗的时间，例如要输出25khz的pwm，1s是10MHZ的tick，一个周期是10mhz/25khz，也就是400tick
        timerConfig_.period_ticks = resolutionHz / pwmFreqHz;
        timerConfig_.count_mode = countMode;
        timerConfig_.flags.update_period_on_empty = updatePeriodOnEmpty;
        timerConfig_.flags.update_period_on_sync = updatePeriodOnSync;
        esp_err_t err = mcpwm_new_timer(&timerConfig_, &timerHandle_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool deinit()
    {
        if(timerHandle_)
        {
            esp_err_t err = mcpwm_del_timer(timerHandle_);
            if(err != ESP_OK)
            {
                printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
                return false;
            }
        }
        return true;
    }
    bool registerEventCb(const mcpwm_timer_event_callbacks_t *cbs, void *user_data)
    {
        esp_err_t err = mcpwm_timer_register_event_callbacks(timerHandle_, cbs, user_data);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool enable()
    {
        esp_err_t err = mcpwm_timer_enable(timerHandle_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool disable()
    {
        esp_err_t err =  mcpwm_timer_disable(timerHandle_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool startOrStop(mcpwm_timer_start_stop_cmd_t command)
    {
        esp_err_t err =  mcpwm_timer_start_stop(timerHandle_, command);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool setPhaseOnSync(mcpwm_sync_handle_t sync_src, uint32_t count_value, mcpwm_timer_direction_t direction)
    {
        mcpwm_timer_sync_phase_config_t syncPhaseCfg = {sync_src, count_value, direction};
        esp_err_t err = mcpwm_timer_set_phase_on_sync(timerHandle_, &syncPhaseCfg);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
private:
    int groupId_;
    mcpwm_timer_config_t timerConfig_;
    mcpwm_timer_handle_t timerHandle_;
};

class McpwmOperator
{
public:
    McpwmOperator(McpwmTimer* timer):timer_(timer)
    {

    }
    ~McpwmOperator()
    {

    }
    McpwmOperator(const McpwmOperator&) = delete;
    McpwmOperator(McpwmOperator&&) = delete;
    McpwmOperator& operator=(const McpwmOperator&) =delete;
    McpwmOperator& operator=(McpwmOperator&&) =delete;
    mcpwm_oper_handle_t getHandle() const
    {
        return operatorHandle_;
    }
    bool init(bool updateGenActionOnTez = false, bool updateGenActionOnTep = false, bool updateGenActionOnSync = false, bool updateDeadTimeOnTez = false, bool updateDeadTimeOnTep = false, bool updateDeadTimeOnSync = false)
    {
        operatorConfig_.group_id = timer_->getGroupId(); /*!< Specify from which group to allocate the MCPWM operator */
        operatorConfig_.flags.update_gen_action_on_tez = updateGenActionOnTez;  /*!< Whether to update generator action when timer counts to zero */
        operatorConfig_.flags.update_gen_action_on_tep = updateGenActionOnTep;  /*!< Whether to update generator action when timer counts to peak */
        operatorConfig_.flags.update_gen_action_on_sync = updateGenActionOnSync; /*!< Whether to update generator action on sync event */
        operatorConfig_.flags.update_dead_time_on_tez = updateDeadTimeOnTez;   /*!< Whether to update dead time when timer counts to zero */
        operatorConfig_.flags.update_dead_time_on_tep = updateDeadTimeOnTep;   /*!< Whether to update dead time when timer counts to peak */
        operatorConfig_.flags.update_dead_time_on_sync = updateDeadTimeOnSync;  /*!< Whether to update dead time on sync event */
        esp_err_t err = mcpwm_new_operator(&operatorConfig_, &operatorHandle_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        err = mcpwm_operator_connect_timer(operatorHandle_, timer_->getHandle());
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool deinit()
    {
        if(operatorHandle_)
        {
            esp_err_t err = mcpwm_del_operator(operatorHandle_);
            if(err != ESP_OK)
            {
                printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
                return false;
            }
        }
        return true;
    }
    McpwmTimer* getTimer()
    {
        return timer_;
    }
private:
    McpwmTimer* timer_;
    mcpwm_operator_config_t operatorConfig_;
    mcpwm_oper_handle_t operatorHandle_;
};

class McpwmComparator
{
public:
    McpwmComparator(McpwmOperator* op) : operator_(op)
    {

    }
    ~McpwmComparator()
    {

    }
    McpwmComparator(const McpwmComparator&) = delete;
    McpwmComparator(McpwmComparator&&) = delete;
    McpwmComparator& operator=(const McpwmComparator&) =delete;
    McpwmComparator& operator=(McpwmComparator&&) =delete;
    mcpwm_cmpr_handle_t getHandle() const
    {
        return cmprHandle_;
    }
    bool init(bool updateCmpOnTez = false, bool updateCmpOnTep = false, bool updateCmpOnSync = false)
    {
        operatorConfig_.flags.update_cmp_on_tez = updateCmpOnTez;
        operatorConfig_.flags.update_cmp_on_tep = updateCmpOnTep;
        operatorConfig_.flags.update_cmp_on_sync = updateCmpOnSync;
        esp_err_t err = mcpwm_new_comparator(operator_->getHandle(), &operatorConfig_, &cmprHandle_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool deinit()
    {
        if(cmprHandle_)
        {
            esp_err_t err = mcpwm_del_comparator(cmprHandle_);
            if(err != ESP_OK)
            {
                printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
                return false;
            }
        }
        return true;
    }
    McpwmOperator* getOperator()
    {
        return operator_;
    }

    bool setCompareVal(uint32_t cmpTicks)
    {
        esp_err_t err = mcpwm_comparator_set_compare_value(cmprHandle_, cmpTicks);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool registerEventCb(const mcpwm_comparator_event_callbacks_t *cbs, void *user_data)
    {
        esp_err_t err = mcpwm_comparator_register_event_callbacks(cmprHandle_, cbs, user_data);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }

private:
    McpwmOperator* operator_;
    mcpwm_comparator_config_t operatorConfig_;
    mcpwm_cmpr_handle_t cmprHandle_;
};

class McpwmGenerator
{
public:
    McpwmGenerator(McpwmOperator* op) : operator_(op)
    {

    }
    ~McpwmGenerator()
    {

    }
    McpwmGenerator(const McpwmGenerator&) = delete;
    McpwmGenerator(McpwmGenerator&&) = delete;
    McpwmGenerator& operator=(const McpwmGenerator&) =delete;
    McpwmGenerator& operator=(McpwmGenerator&&) =delete;
    mcpwm_gen_handle_t getHandle() const
    {
        return genHandle_;
    }
    bool init(gpio_num_t gpioNum, bool invertPwm = true, bool ioLoopBack = false)
    {
        generatorConfig_.gen_gpio_num = gpioNum;           /*!< The GPIO number used to output the PWM signal */
        generatorConfig_.flags.invert_pwm = invertPwm;   /*!< Whether to invert the PWM signal (done by GPIO matrix) */
        generatorConfig_.flags.io_loop_back = ioLoopBack; /*!< For debug/test, the signal output from the GPIO will be fed to the input path as well */
        esp_err_t err = mcpwm_new_generator(operator_->getHandle(), &generatorConfig_, &genHandle_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool deinit()
    {
        if(genHandle_)
        {
            esp_err_t err = mcpwm_del_generator(genHandle_);
            if(err != ESP_OK)
            {
                printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
                return false;
            }
        }
        return true;
    }
    McpwmOperator* getOperator()
    {
        return operator_;
    }
    bool setForceLevel(int level, bool holdOn)
    {
        esp_err_t err = mcpwm_generator_set_force_level(genHandle_, level, holdOn);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool setActionsOnTimerEvent(mcpwm_gen_timer_event_action_t evAct, mcpwm_gen_timer_event_action_t end)
    {
        esp_err_t err = mcpwm_generator_set_actions_on_timer_event(genHandle_, evAct, end);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool setActionsOnCompareEvent(mcpwm_gen_compare_event_action_t evAct, mcpwm_gen_compare_event_action_t end)
    {
        esp_err_t err = mcpwm_generator_set_actions_on_compare_event(genHandle_, evAct, end);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool setActionOnBrakeEvent(mcpwm_gen_brake_event_action_t evAct, mcpwm_gen_brake_event_action_t end)
    {
        esp_err_t err = mcpwm_generator_set_actions_on_brake_event(genHandle_, evAct, end);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool setDeadTime()
    {
        /*esp_err_t err = mcpwm_generator_set_dead_time(mcpwm_gen_handle_t in_generator, mcpwm_gen_handle_t out_generator, const mcpwm_dead_time_config_t *config);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }*/
        return true;
    }

private:
    McpwmOperator* operator_;
    mcpwm_generator_config_t generatorConfig_;
    mcpwm_gen_handle_t genHandle_;
};