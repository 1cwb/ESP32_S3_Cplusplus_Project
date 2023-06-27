#include "mrmt.h"
#include <cstring>

RmtEncoderBase::RmtEncoderBase() : copyEncoder_(nullptr), bytesEncoder_(nullptr),state_(0)
{
    memset(&base_, 0, sizeof(rmt_encoder_t));
}
RmtEncoderBase::~RmtEncoderBase()
{
    deInitBytesEcoder();
    deInitCopyEcoder();
}
int RmtEncoderBase::getState() const
{
    return state_;
}
void RmtEncoderBase::setState(int state)
{
    state_ = state;
}
bool RmtEncoderBase::initBytesEcoder(const rmt_bytes_encoder_config_t *config)
{
    esp_err_t err = ESP_FAIL;
    if(config)
    {
        memcpy(&bytesEncoderConfig_, config, sizeof(rmt_bytes_encoder_config_t));
        err = rmt_new_bytes_encoder(&bytesEncoderConfig_, &bytesEncoder_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    else
    {
        return false;
    }
}
bool RmtEncoderBase::initCopyEcoder( const rmt_copy_encoder_config_t *cpoyConfig)
{
    esp_err_t err = ESP_FAIL;
    if(cpoyConfig)
    {
        memcpy(&copyEncoderConfig_, cpoyConfig, sizeof(rmt_copy_encoder_config_t));
        err = rmt_new_copy_encoder(&copyEncoderConfig_,  &copyEncoder_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    else
    {
        return false;
    }
}
bool RmtEncoderBase::deInitBytesEcoder()
{
    if(bytesEncoder_)
    {
        esp_err_t err = rmt_del_encoder(bytesEncoder_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    else
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,"bytesEncoder_ is nullptr!");
        return false;
    }
}
bool RmtEncoderBase::deInitCopyEcoder()
{
    if(copyEncoder_)
    {
        esp_err_t err = rmt_del_encoder(copyEncoder_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    else
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,"copyEncoder_ is nullptr!");
        return false;
    }
}
bool RmtEncoderBase::resetBytesEcoder()
{
    if(bytesEncoder_)
    {
        esp_err_t err = rmt_encoder_reset(bytesEncoder_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        }
        return true; 
    }
    else
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,"bytesEncoder_ is nullptr!");
        return false;
    }
}
bool RmtEncoderBase::resetCopyEcoder()
{
    if(copyEncoder_)
    {
        esp_err_t err = rmt_encoder_reset(copyEncoder_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        }
        return true; 
    }
    else
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,"copyEncoder_ is nullptr!");
        return false;
    }
}
rmt_encoder_handle_t RmtEncoderBase::getEncoderHandle()
{
    return &base_;
}
rmt_encoder_handle_t RmtEncoderBase::getBytesEncoder()
{
    return bytesEncoder_;
}
rmt_encoder_handle_t RmtEncoderBase::getCopyEncoder()
{
    return copyEncoder_;
}
void RmtEncoderBase::setBaseEncoderCb(encoderCallback cb)
{
    base_.encode = cb;
}
void RmtEncoderBase::setBaseResetCb(resetCallback cb)
{
    base_.reset = cb;
}
void RmtEncoderBase::setBaseDelCb(delCallback cb)
{
    base_.del = cb;
}

//////////////////////////////////RmtBase///////////////////////////////////////////
RmtBase::RmtBase():binit_(false)
{

}
RmtBase::~RmtBase()
{
    if(handle_)
    {
        deInit();
    }
}

bool RmtBase::init(rmt_channel_handle_t handle)
{
    handle_ = handle;
    binit_ = true;
    return true;
}
bool RmtBase::deInit()
{
    esp_err_t err = rmt_del_channel(handle_);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    binit_ = false;
    return true;
}
bool RmtBase::applyCarrier(uint32_t frequencyHz, float dutyCycle, bool alwaysOn, bool polarityActiveLow)
{
    txCarrierCfg_.frequency_hz = frequencyHz;
    txCarrierCfg_.duty_cycle = dutyCycle;
    txCarrierCfg_.flags.polarity_active_low = polarityActiveLow;
    txCarrierCfg_.flags.always_on = alwaysOn;
    esp_err_t err = rmt_apply_carrier(handle_, &txCarrierCfg_);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}
bool RmtBase::enable()
{
    esp_err_t err = rmt_enable(handle_);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}
bool RmtBase::disable()
{
    esp_err_t err = rmt_disable(handle_);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}

///////////////////////////////mrmttx
bool MRmtTx::init(gpio_num_t gpio, uint32_t resolutionHz , size_t memBlockSymbols, size_t transQueueDepth ,bool invertOut, bool withDma, bool ioLoopBack, bool ioOdMode, rmt_clock_source_t clkSrc)
{
    rmt_channel_handle_t handle;
    txChanelCfg_.gpio_num = gpio;          
    txChanelCfg_.clk_src = clkSrc;  
    txChanelCfg_.resolution_hz = resolutionHz;             
    txChanelCfg_.mem_block_symbols = memBlockSymbols;         
    txChanelCfg_.trans_queue_depth = transQueueDepth;         

    txChanelCfg_.flags.invert_out = invertOut;  
    txChanelCfg_.flags.with_dma = withDma;    
    txChanelCfg_.flags.io_loop_back = ioLoopBack;
    txChanelCfg_.flags.io_od_mode = ioOdMode;
    esp_err_t err = rmt_new_tx_channel(&txChanelCfg_, &handle);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    RmtBase::init(handle);
    return true;
}
bool MRmtTx::registerTxEventCb(const rmt_tx_event_callbacks_t *cbs, void *userData)
{
    esp_err_t err = rmt_tx_register_event_callbacks(getHandle(), cbs, userData);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}
bool MRmtTx::RemoveRegisterTxEventCb()
{
    return registerTxEventCb(nullptr, nullptr);
}
bool MRmtTx::transmit(rmt_encoder_t *encoder, const void *payload, size_t payloadBytes, const rmt_transmit_config_t *config)
{
    esp_err_t err = rmt_transmit(getHandle(), encoder, payload, payloadBytes, config);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}
bool MRmtTx::txWaitAllDone(int timeoutMs)
{
    esp_err_t err = rmt_tx_wait_all_done(getHandle(), timeoutMs);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}
//////////////mrmtrx//////////////////////////////////////////////
bool MRmtRx::init(gpio_num_t gpio, uint32_t resolutionHz , size_t memBlockSymbols ,bool invertIn, bool withDma, bool ioLoopBack, bool ioOdMode, rmt_clock_source_t clkSrc)
{
    rmt_channel_handle_t handle;

    rxChanelCfg_.gpio_num = gpio;
    rxChanelCfg_.clk_src = clkSrc;  
    rxChanelCfg_.resolution_hz = resolutionHz;             
    rxChanelCfg_.mem_block_symbols = memBlockSymbols;               

    rxChanelCfg_.flags.invert_in = invertIn;  
    rxChanelCfg_.flags.with_dma = withDma;    
    rxChanelCfg_.flags.io_loop_back = ioLoopBack;
    esp_err_t err = rmt_new_rx_channel(&rxChanelCfg_, &handle);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    RmtBase::init(handle);
    return true;
}
bool MRmtRx::registerRxEventCb(const rmt_rx_event_callbacks_t *cbs, void *userData)
{
    esp_err_t err = rmt_rx_register_event_callbacks(getHandle(), cbs, userData);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}
bool MRmtRx::RemoveRegisterRxEventCb()
{
    return registerRxEventCb(nullptr, nullptr);
}
bool MRmtRx::receive(void *buffer, size_t bufferSize, const rmt_receive_config_t *config)
{
    esp_err_t err = rmt_receive(getHandle(), buffer, bufferSize, config);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}