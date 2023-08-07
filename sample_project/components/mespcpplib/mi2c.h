#pragma once
#include "driver/i2c.h"

class MI2CMaster
{
public:
    MI2CMaster(int32_t sdaPinNum, int32_t sclPinNum, i2c_port_t i2cNum = I2C_NUM_0, uint32_t clockSpeed = 1000000) : i2cMasterPort_(i2cNum), config_(nullptr)
    {
        config_ = new i2c_config_t;
        if(!config_)
        {
            printf("error: %s()%d new i2c_config_t fail\n",__FUNCTION__,__LINE__);
            return;
        }
        memset(config_, 0, sizeof(i2c_config_t));
        config_->mode = I2C_MODE_MASTER;
        config_->sda_io_num = sdaPinNum;
        config_->scl_io_num = sclPinNum;
        config_->sda_pullup_en = GPIO_PULLUP_ENABLE;
        config_->scl_pullup_en = GPIO_PULLUP_ENABLE;
        config_->master.clk_speed = clockSpeed;
        init();
    }
    ~MI2CMaster()
    {
        deinit();
        if(config_)
        {
            delete config_;
            config_ = nullptr;
        }
    }
    MI2CMaster(const MI2CMaster&) = delete;
    MI2CMaster(MI2CMaster&&) = delete;
    MI2CMaster& operator=(const MI2CMaster&) =delete;
    MI2CMaster& operator=(MI2CMaster&&) =delete;
    bool init()
    {
        esp_err_t err = ESP_OK;
        err = i2c_param_config(i2cMasterPort_, config_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        err = i2c_driver_install(i2cMasterPort_, config_->mode, false, false, false);//master mode do not need buf
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool deinit()
    {
        esp_err_t err = i2c_driver_delete(i2cMasterPort_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool writeToDevice(uint8_t deviceAddress, const uint8_t* writeBuffer, size_t writeSize, TickType_t ticksToWait)
    {
        esp_err_t err = i2c_master_write_to_device(i2cMasterPort_, deviceAddress, writeBuffer, writeSize, ticksToWait);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool readFromDevice(uint8_t deviceAddress, uint8_t* readBuffer, size_t readSize, TickType_t ticksToWait)
    {
        esp_err_t err = i2c_master_read_from_device(i2cMasterPort_, deviceAddress, readBuffer, readSize, ticksToWait);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool writeReadDevice(uint8_t deviceAddress, const uint8_t* writeBuffer, size_t writeSize, uint8_t* read_buffer, size_t read_size, TickType_t ticksToWait)
    {
        esp_err_t err = i2c_master_write_read_device(i2cMasterPort_, deviceAddress, writeBuffer, writeSize, read_buffer, read_size, ticksToWait);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
private:
    i2c_port_t i2cMasterPort_;// = I2C_MASTER_NUM;
    i2c_config_t* config_;
};