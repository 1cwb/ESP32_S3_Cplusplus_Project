#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_timer.h"

class MSpiDevice
{
public:
    MSpiDevice() = default;
    void init(int32_t clockSpeedHz, int32_t spiCsNum , transaction_cb_t preCb = nullptr, transaction_cb_t postCb = nullptr, uint8_t mode = 0, int32_t queueSize = 7)
    {
        devCfg_.command_bits = 0;
        devCfg_.address_bits = 0;
        devCfg_.dummy_bits = 0;
        devCfg_.mode = mode;
        devCfg_.duty_cycle_pos = 128;
        devCfg_.cs_ena_pretrans = 0;       
        devCfg_.cs_ena_posttrans = 0;      
        devCfg_.clock_speed_hz = clockSpeedHz; //SPI_MASTER_FREQ_26M         
        devCfg_.input_delay_ns = 0;

        devCfg_.spics_io_num = spiCsNum;
        devCfg_.flags = 0;
        devCfg_.queue_size = queueSize;
        devCfg_.pre_cb = preCb;
        devCfg_.post_cb = postCb;
    }
    ~MSpiDevice() = default;
    MSpiDevice(const MSpiDevice&) = delete;
    MSpiDevice(MSpiDevice&&) = delete;
    MSpiDevice& operator=(const MSpiDevice& ) = delete;
    MSpiDevice& operator=(MSpiDevice&& ) = delete;
    spi_device_handle_t* getHandle() {return &handle_;}
    spi_device_interface_config_t* getDevCfg() {return &devCfg_;}
    
    bool pollingTransmit(const uint8_t *data, int32_t len, void* userData)
    {
        memset(&transDesc_, 0, sizeof(spi_transaction_t));
        transDesc_.length = len * 8;
        transDesc_.tx_buffer = data;
        transDesc_.user = userData;
        esp_err_t err = spi_device_polling_transmit(handle_, &transDesc_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool pollingEnd(TickType_t ticksTowait)
    {
        esp_err_t err = spi_device_polling_end(handle_, ticksTowait);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool pollingStart(const uint8_t *data, int32_t len, void* userData, TickType_t ticksTowait)
    {
        memset(&transDesc_, 0, sizeof(spi_transaction_t));
        transDesc_.length = len * 8;
        transDesc_.tx_buffer = data;
        transDesc_.user = userData;
        esp_err_t err = spi_device_polling_start(handle_, &transDesc_, ticksTowait);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool transmit(const uint8_t *data, int32_t len, uint8_t *recvData, int32_t rxLen,void* userData)
    {
        memset(&transDesc_, 0, sizeof(spi_transaction_t));
        transDesc_.length = len * 8;
        if(rxLen > len)
        {
            rxLen = len;
        }
        transDesc_.length = len * 8;
        transDesc_.tx_buffer = data;
        transDesc_.rx_buffer = recvData;
        transDesc_.rxlength = rxLen * 8;
        transDesc_.user = userData;
        esp_err_t err = spi_device_transmit(handle_, &transDesc_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
private:
    spi_device_handle_t handle_;
    spi_device_interface_config_t devCfg_;
    spi_transaction_t transDesc_;
};

class MSpiBus
{
public:
    MSpiBus() = default;
    ~MSpiBus() = default;
    MSpiBus(const MSpiBus&) = delete;
    MSpiBus(MSpiBus&&) = delete;
    MSpiBus& operator=(const MSpiBus& ) = delete;
    MSpiBus& operator=(MSpiBus&& ) = delete;
    bool spiBusInit(int32_t mosiIoNum, int32_t misoIoNum, int32_t sclkIoNum, spi_host_device_t hostId, spi_dma_chan_t dmaCh = SPI_DMA_CH_AUTO, int32_t maxTransfSz = 4096);
    bool spiBusDeinit();
    bool addDevice(MSpiDevice* dev);
    bool removeDevice(MSpiDevice* dev);
    spi_host_device_t getHostId() const {return hostId_;}
private:
    spi_host_device_t hostId_;
    spi_bus_config_t busCfg_;
    MSpiDevice* dev_;
};