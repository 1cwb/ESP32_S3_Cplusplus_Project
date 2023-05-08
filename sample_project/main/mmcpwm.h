#pragma once

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/mcpwm_prelude.h"

struct stBdcMotorMcpwmObj{
    mcpwm_timer_handle_t timerHandle_;
    mcpwm_oper_handle_t operatorHandle_;
    mcpwm_cmpr_handle_t cmpAHandle_;
    mcpwm_cmpr_handle_t cmpBHandle_;
    mcpwm_gen_handle_t genAHandle_;
    mcpwm_gen_handle_t genBHandle_;

    mcpwm_timer_config_t timerConfig_;
    mcpwm_operator_config_t operatorConfig_;
    mcpwm_comparator_config_t comparatorConfig_;
    mcpwm_generator_config_t generatorAConfig_;
    mcpwm_generator_config_t generatorBConfig_;
};

class MMcpwm
{
public:
    MMcpwm():groupId_(0),pMcpwmObj_(nullptr)
    {
        if(!pMcpwmObj_)
        {
            pMcpwmObj_ = static_cast<stBdcMotorMcpwmObj*>(malloc(sizeof(stBdcMotorMcpwmObj)));
        }
    }
    ~MMcpwm()
    {
        if(pMcpwmObj_)
        {
            deinit();
            free(pMcpwmObj_);
            pMcpwmObj_ = nullptr;
        }
    }
    MMcpwm(const MMcpwm&) = delete;
    MMcpwm(MMcpwm&&) = delete;
    MMcpwm& operator=(const MMcpwm&) =delete;
    MMcpwm& operator=(MMcpwm&&) =delete;
    bool init(int groupId, gpio_num_t pwmaGpioNum, gpio_num_t pwmbGpioNum, uint32_t pwmFreqHz, uint32_t resolutionHz)
    {
        if(!pMcpwmObj_)
        {
            return false;
        }
        groupId_ = groupId;
        esp_err_t err = ESP_OK;
        do{
            pMcpwmObj_->timerConfig_.group_id = groupId;
            pMcpwmObj_->timerConfig_.clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT;
            pMcpwmObj_->timerConfig_.resolution_hz = resolutionHz;
            pMcpwmObj_->timerConfig_.period_ticks = resolutionHz / pwmFreqHz;
            pMcpwmObj_->timerConfig_.count_mode = MCPWM_TIMER_COUNT_MODE_UP;
            err = mcpwm_new_timer(&pMcpwmObj_->timerConfig_, &pMcpwmObj_->timerHandle_);
            if(err != ESP_OK)
            {
                printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
                break;
            }

            pMcpwmObj_->operatorConfig_.group_id = groupId;
            err = mcpwm_new_operator(&pMcpwmObj_->operatorConfig_, &pMcpwmObj_->operatorHandle_);
            if(err != ESP_OK)
            {
                printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
                break;
            }
            err = mcpwm_operator_connect_timer(pMcpwmObj_->operatorHandle_, pMcpwmObj_->timerHandle_);
            if(err != ESP_OK)
            {
                printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
                break;
            }

            pMcpwmObj_->comparatorConfig_.flags.update_cmp_on_tez = true;
            err = mcpwm_new_comparator(pMcpwmObj_->operatorHandle_, &pMcpwmObj_->comparatorConfig_, &pMcpwmObj_->cmpAHandle_);
            if(err != ESP_OK)
            {
                printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
                break;
            }
            err = mcpwm_new_comparator(pMcpwmObj_->operatorHandle_, &pMcpwmObj_->comparatorConfig_, &pMcpwmObj_->cmpBHandle_);
            if(err != ESP_OK)
            {
                printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
                break;
            }
            // set the initial compare value for both comparators
            mcpwm_comparator_set_compare_value(pMcpwmObj_->cmpBHandle_, 0);
            mcpwm_comparator_set_compare_value(pMcpwmObj_->cmpBHandle_, 0);

            pMcpwmObj_->generatorAConfig_.gen_gpio_num = static_cast<int>(pwmaGpioNum);
            err = mcpwm_new_generator(pMcpwmObj_->operatorHandle_, &pMcpwmObj_->generatorAConfig_,&pMcpwmObj_->genAHandle_);
            if(err != ESP_OK)
            {
                printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
                break;
            }
            pMcpwmObj_->generatorBConfig_.gen_gpio_num = static_cast<int>(pwmbGpioNum);
            err = mcpwm_new_generator(pMcpwmObj_->operatorHandle_, &pMcpwmObj_->generatorBConfig_,&pMcpwmObj_->genBHandle_);
            if(err != ESP_OK)
            {
                printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
                break;
            }
            mcpwm_gen_timer_event_action_t timerEventActionEnd = {.direction = MCPWM_TIMER_DIRECTION_UP, .event = MCPWM_TIMER_EVENT_INVALID, .action = MCPWM_GEN_ACTION_HIGH};
            mcpwm_gen_compare_event_action_t compareEventActionEnd = {.direction = MCPWM_TIMER_DIRECTION_UP, .comparator = nullptr, .action = MCPWM_GEN_ACTION_LOW};
            mcpwm_generator_set_actions_on_timer_event(pMcpwmObj_->genAHandle_,MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH),timerEventActionEnd);
            mcpwm_generator_set_actions_on_compare_event(pMcpwmObj_->genAHandle_,MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, pMcpwmObj_->cmpAHandle_, MCPWM_GEN_ACTION_LOW),compareEventActionEnd);
            mcpwm_generator_set_actions_on_timer_event(pMcpwmObj_->genBHandle_,MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH),timerEventActionEnd);
            mcpwm_generator_set_actions_on_compare_event(pMcpwmObj_->genBHandle_,MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, pMcpwmObj_->cmpBHandle_, MCPWM_GEN_ACTION_LOW),compareEventActionEnd);
            return true;
        }while(false);
        if(pMcpwmObj_)
        {
            if(pMcpwmObj_->genAHandle_)
            {
                mcpwm_del_generator(pMcpwmObj_->genAHandle_);
                pMcpwmObj_->genAHandle_ = nullptr;
            }
            if(pMcpwmObj_->genBHandle_)
            {
                mcpwm_del_generator(pMcpwmObj_->genBHandle_);
                pMcpwmObj_->genBHandle_ = nullptr;
            }
            if(pMcpwmObj_->cmpAHandle_)
            {
                mcpwm_del_comparator(pMcpwmObj_->cmpAHandle_);
                pMcpwmObj_->cmpAHandle_ = nullptr;
            }
            if(pMcpwmObj_->cmpBHandle_)
            {
                mcpwm_del_comparator(pMcpwmObj_->cmpBHandle_);
                pMcpwmObj_->cmpBHandle_ = nullptr;
            }
            if(pMcpwmObj_->operatorHandle_)
            {
                mcpwm_del_operator(pMcpwmObj_->operatorHandle_);
                pMcpwmObj_->operatorHandle_ = nullptr;
            }
            if(pMcpwmObj_->timerHandle_)
            {
                mcpwm_del_timer(pMcpwmObj_->timerHandle_);
                pMcpwmObj_->timerHandle_ = nullptr;
            }
        }
        return false;
    }
    bool deinit()
    {
        if(!pMcpwmObj_)
        {
            return false;
        }
             if(pMcpwmObj_->genAHandle_)
            {
                mcpwm_del_generator(pMcpwmObj_->genAHandle_);
                pMcpwmObj_->genAHandle_ = nullptr;
            }
            if(pMcpwmObj_->genBHandle_)
            {
                mcpwm_del_generator(pMcpwmObj_->genBHandle_);
                pMcpwmObj_->genBHandle_ = nullptr;
            }
            if(pMcpwmObj_->cmpAHandle_)
            {
                mcpwm_del_comparator(pMcpwmObj_->cmpAHandle_);
                pMcpwmObj_->cmpAHandle_ = nullptr;
            }
            if(pMcpwmObj_->cmpBHandle_)
            {
                mcpwm_del_comparator(pMcpwmObj_->cmpBHandle_);
                pMcpwmObj_->cmpBHandle_ = nullptr;
            }
            if(pMcpwmObj_->operatorHandle_)
            {
                mcpwm_del_operator(pMcpwmObj_->operatorHandle_);
                pMcpwmObj_->operatorHandle_ = nullptr;
            }
            if(pMcpwmObj_->timerHandle_)
            {
                mcpwm_del_timer(pMcpwmObj_->timerHandle_);
                pMcpwmObj_->timerHandle_ = nullptr;
            }
        return true;
    }
    bool setSpeed(uint32_t speed)
    {
        esp_err_t err = mcpwm_comparator_set_compare_value(pMcpwmObj_->cmpAHandle_, speed);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        err = mcpwm_comparator_set_compare_value(pMcpwmObj_->cmpBHandle_, speed);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool enable()
    {
        esp_err_t err = mcpwm_timer_enable(pMcpwmObj_->timerHandle_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        err = mcpwm_timer_start_stop(pMcpwmObj_->timerHandle_, MCPWM_TIMER_START_NO_STOP);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool disable()
    {
        esp_err_t err = mcpwm_timer_start_stop(pMcpwmObj_->timerHandle_, MCPWM_TIMER_STOP_EMPTY);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        err = mcpwm_timer_enable(pMcpwmObj_->timerHandle_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool forward()
    {
        esp_err_t err = mcpwm_generator_set_force_level(pMcpwmObj_->genAHandle_, -1, true);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        err = mcpwm_generator_set_force_level(pMcpwmObj_->genBHandle_, 0, true);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool reverse()
    {
        esp_err_t err = mcpwm_generator_set_force_level(pMcpwmObj_->genAHandle_, 0, true);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        err = mcpwm_generator_set_force_level(pMcpwmObj_->genBHandle_, -1, true);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool coast()
    {
        esp_err_t err = mcpwm_generator_set_force_level(pMcpwmObj_->genAHandle_, 0, true);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        err = mcpwm_generator_set_force_level(pMcpwmObj_->genBHandle_, 0, true);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool brake()
    {
        esp_err_t err = mcpwm_generator_set_force_level(pMcpwmObj_->genAHandle_, 1, true);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        err = mcpwm_generator_set_force_level(pMcpwmObj_->genBHandle_, 1, true);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
private:
    int groupId_;
    stBdcMotorMcpwmObj* pMcpwmObj_;
};

