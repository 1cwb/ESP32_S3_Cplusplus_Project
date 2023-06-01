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

#define ESP_SPI_MUTEX_TICKS_TO_WAIT 2

#define SPI_DEVICE_MUTEX_TAKE(mutex, ret) if (!xSemaphoreTake(mutex, ESP_SPI_MUTEX_TICKS_TO_WAIT)) { \
        printf("%s()%d spi device take mutex timeout, max wait = %d ticks",__FUNCTION__,__LINE__, ESP_SPI_MUTEX_TICKS_TO_WAIT); \
        return (ret); \
    }

#define SPI_DEVICE_MUTEX_GIVE(mutex, ret) if (!xSemaphoreGive(mutex)) { \
        printf("%s()%d spi device give mutex failed",__FUNCTION__,__LINE__); \
        return (ret); \
    }

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
        mutex_ = xSemaphoreCreateMutex();
    }
    void init(spi_device_interface_config_t devcfg)
    {
        memcpy(&devCfg_, &devcfg, sizeof(spi_device_interface_config_t));
        mutex_ = xSemaphoreCreateMutex();
    }
    void deinit()
    {
        vSemaphoreDelete(mutex_);
    }
    ~MSpiDevice() = default;
    MSpiDevice(const MSpiDevice&) = delete;
    MSpiDevice(MSpiDevice&&) = delete;
    MSpiDevice& operator=(const MSpiDevice& ) = delete;
    MSpiDevice& operator=(MSpiDevice&& ) = delete;
    spi_device_handle_t* getHandle() {return &handle_;}
    spi_device_interface_config_t* getDevCfg() {return &devCfg_;}

    bool pollingTransmit(spi_transaction_t* transDesc)
    {
        esp_err_t err = spi_device_polling_transmit(handle_, transDesc);
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
    bool pollingStart(spi_transaction_t* transDesc, TickType_t ticksTowait)
    {
        esp_err_t err = spi_device_polling_start(handle_, transDesc, ticksTowait);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool transmit(spi_transaction_t *transDesc)
    {
        esp_err_t err = spi_device_transmit(handle_, transDesc);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    
    bool queueTrans(spi_transaction_t *transDesc, TickType_t ticksToWait)
    {
        esp_err_t err = spi_device_queue_trans(handle_, transDesc, ticksToWait);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool getTranseResult(spi_transaction_t **transDesc, TickType_t ticksToWait)
    {
        esp_err_t err = spi_device_get_trans_result(handle_, transDesc, ticksToWait);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool acquireBus(TickType_t wait)
    {
        esp_err_t err = spi_device_acquire_bus(handle_, wait);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    void releaseBus()
    {
        spi_device_release_bus(handle_);
    }
    SemaphoreHandle_t* getMutex()
    {
        return &mutex_;
    }

    bool transferByte(uint8_t dataOut, uint8_t *dataIn, void* user)
    {
        esp_err_t err;
        spi_transaction_t trans;
        memset(&trans, 0, sizeof(spi_transaction_t));
        trans.length = 8;
        trans.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
        trans.tx_data[0] = dataOut;
        trans.user = user;
        err = spiDevicePollingTransmit(&trans);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }

        if (dataIn) {
            *dataIn = trans.rx_data[0];
        }

        return true;
    }
    bool transferBytes(const uint8_t *dataOut, uint8_t *dataIn, uint32_t dataLen, void* user)
    {
        esp_err_t err;
        spi_transaction_t trans;
        memset(&trans, 0, sizeof(spi_transaction_t));
        trans.length = dataLen * 8;
        trans.tx_buffer = nullptr;
        trans.rx_buffer = nullptr;
        trans.user = user;
        if (dataOut) {
            trans.tx_buffer = dataOut;
        }

        if (dataIn) {
            trans.rx_buffer = dataIn;
        }

        err = spiDevicePollingTransmit(&trans);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }

        return true;
    }
    bool transmitBegin(spi_transaction_t *ptrans)
    {
        return (spiDevicePollingTransmit(ptrans) == ESP_OK);
    }

    bool transferReg16( uint16_t dataOut, uint16_t *dataIn, void* user)
    {
        esp_err_t err;
        spi_transaction_t trans;
        memset(&trans, 0, sizeof(spi_transaction_t));
        trans.length = 16;
        trans.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
        /* default MSB first */
        trans.tx_data[0] = (dataOut >> 8) & 0xff;
        trans.tx_data[1] = dataOut & 0xff;
        trans.user = user;
        err = spiDevicePollingTransmit(&trans);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }

        if (dataIn) {
            *dataIn = (trans.rx_data[0] << 8) | (trans.rx_data[1]);
        }

        return true;
    }

    bool transferReg32(uint32_t dataOut, uint32_t *dataIn, void* user)
    {
        esp_err_t err;
        spi_transaction_t trans;
        memset(&trans, 0, sizeof(spi_transaction_t));
        trans.length = 32;
        trans.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
        /* default MSB first */

        trans.tx_data[0] = (dataOut >> 24) & 0xff;
        trans.tx_data[1] = (dataOut >> 16) & 0xff;
        trans.tx_data[2] = (dataOut >> 8) & 0xff;
        trans.tx_data[3] = dataOut & 0xff;
        trans.user = user;
        err = spiDevicePollingTransmit(&trans);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }

        if (dataIn) {
            *dataIn = (trans.rx_data[0] << 24) | (trans.rx_data[1] << 16) | (trans.rx_data[2] << 8) | (trans.rx_data[3]);
        }
        return true;
    }
private:

    /* this function should lable with inline*/
    inline esp_err_t spiDevicePollingTransmit(spi_transaction_t *trans)
    {
        esp_err_t ret;
        SPI_DEVICE_MUTEX_TAKE(mutex_, ESP_FAIL);
        ret = spi_device_polling_transmit(handle_, trans);
        SPI_DEVICE_MUTEX_GIVE(mutex_, ESP_FAIL);
        return ret;
    }
private:
    spi_device_handle_t handle_;
    spi_device_interface_config_t devCfg_;
    SemaphoreHandle_t mutex_;
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