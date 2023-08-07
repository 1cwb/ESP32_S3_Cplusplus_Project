#pragma once
#include "driver/uart.h"
#include <thread>
#include <functional>

class MUart
{
    using UartEventCallBack = std::function<void(uart_event_t*)>;
public:
    MUart(int32_t uartNum = UART_NUM_1, int32_t baudrate = 115200, uart_word_length_t dataBits = UART_DATA_8_BITS, uart_parity_t parity = UART_PARITY_DISABLE, uart_stop_bits_t stopBits = UART_STOP_BITS_1, uart_hw_flowcontrol_t flowCtrl = UART_HW_FLOWCTRL_DISABLE) : uartNum_(uartNum), config_(nullptr), uartQueue_(nullptr)
    {
        config_ = new uart_config_t;
        if(!config_)
        {
            printf("error: new uart_config_t fail\n");
            return;
        }
        cb_ = new UartEventCallBack;
        memset(config_, 0, sizeof(uart_config_t));
        config_->baud_rate = baudrate;
        config_->data_bits = dataBits;
        config_->parity    = parity;
        config_->stop_bits = stopBits;
        config_->flow_ctrl = flowCtrl;
        config_->source_clk = UART_SCLK_DEFAULT;
    }
    ~MUart()
    {
        if(config_)
        {
            delete config_;
            config_ = nullptr;
        }
        if(cb_)
        {
            delete cb_;
            cb_ = nullptr;
        }
    }
    MUart(const MUart&) = delete;
    MUart(MUart&&) = delete;
    MUart& operator=(const MUart&) =delete;
    MUart& operator=(MUart&&) =delete;
    bool init(int32_t txdPinNum, int32_t rxdPinNum, int32_t rxbufSize = 256, int32_t txbufSize = 256)
    {
        if(uart_is_driver_installed(uartNum_))
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,"already inited");
            return false;
        }
        esp_err_t err = uart_driver_install(uartNum_, rxbufSize, txbufSize, 20, &uartQueue_, 0);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        err = uart_param_config(uartNum_, config_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        err = uart_set_pin(uartNum_, txdPinNum, rxdPinNum, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        std::thread uartthread([this](){
            uart_event_t event;
            while(true)
            {
                if(xQueueReceive(this->uartQueue_, (void * )&event, (TickType_t)portMAX_DELAY))
                {
                    if(event.type == UART_FIFO_OVF || event.type == UART_BUFFER_FULL)
                    {
                        printf("Waring: %s\n",event.type == UART_FIFO_OVF ? "ring buffer overflow!" : "ring buffer full!");
                        uart_flush_input(uartNum_);
                        xQueueReset(uartQueue_);
                    }
                    else if(event.type == UART_BREAK)
                    {
                        printf("Event Error : uart rx break\n");
                    }
                    else if(event.type == UART_PARITY_ERR)
                    {
                        printf("Event Error : uart parity error\n");
                    }
                    else if(event.type == UART_FRAME_ERR)
                    {
                        printf("Event Error : uart frame error\n");
                    }
                    else
                    {
                        if(cb_ && *cb_)
                        {
                            (*cb_)(&event);
                        }
                    }
                }
            }
        });
        uartthread.detach();
        return true;
    }
    bool deinit()
    {
        esp_err_t err = uart_driver_delete(uartNum_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    size_t sendData(const char *data, size_t size)
    {
        return uart_write_bytes(uartNum_, data, size);
    }
    size_t recvData(void *buf, uint32_t length, TickType_t ticks_to_wait)
    {
        return uart_read_bytes(uartNum_, buf, length, ticks_to_wait);
    }
    bool flushInput()
    {
        esp_err_t err = uart_flush_input(uartNum_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool enablePatternDetBaudIntr(char patternChr, uint8_t chrNum, int chrTout, int postIdle, int preIdle)
    {
        esp_err_t err = uart_enable_pattern_det_baud_intr(uartNum_, patternChr, chrNum, chrTout, postIdle, preIdle);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool disablePatternDetBaudIntr()
    {
        esp_err_t err = uart_disable_pattern_det_intr(uartNum_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool getBuffDataLen(size_t *size)
    {
        esp_err_t err = uart_get_buffered_data_len(uartNum_, size);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool getBuffFreeSize(size_t *size)
    {
        esp_err_t err = uart_get_tx_buffer_free_size(uartNum_, size);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    int32_t patternPopPos()
    {
        return uart_pattern_pop_pos(uartNum_);
    }
    int32_t patternGetPos()
    {
        return uart_pattern_get_pos(uartNum_);
    }
    void registerEventCallback(const UartEventCallBack& cb)
    {
        if(cb_)
        {
            *cb_ = cb;
        }
    }
private:
    int32_t uartNum_;
    uart_config_t* config_;
    QueueHandle_t uartQueue_;
    UartEventCallBack* cb_;
};