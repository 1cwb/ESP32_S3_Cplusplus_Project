#pragma once
#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"
#include "driver/gpio.h"

class RmtEncoderBase
{
    using encoderCallback = size_t (*)(rmt_encoder_t *encoder, rmt_channel_handle_t tx_channel, const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state);
    using resetCallback = esp_err_t (*)(rmt_encoder_t *encoder);
    using delCallback = esp_err_t (*)(rmt_encoder_t *encoder);
public:
    RmtEncoderBase();
    ~RmtEncoderBase();
    int getState() const;
    void setState(int state);
    bool initBytesEcoder(const rmt_bytes_encoder_config_t *config);
    bool initCopyEcoder( const rmt_copy_encoder_config_t *cpoyConfig);
    bool deInitBytesEcoder();
    bool deInitCopyEcoder();
    bool resetBytesEcoder();
    bool resetCopyEcoder();
    rmt_encoder_handle_t getEncoderHandle();
    rmt_encoder_handle_t getBytesEncoder();
    rmt_encoder_handle_t getCopyEncoder();
    void setBaseEncoderCb(encoderCallback cb);
    void setBaseResetCb(resetCallback cb);
    void setBaseDelCb(delCallback cb);
private:
    rmt_encoder_t base_;
    rmt_encoder_t *copyEncoder_;
    rmt_encoder_t *bytesEncoder_;
    rmt_bytes_encoder_config_t bytesEncoderConfig_;
    rmt_copy_encoder_config_t copyEncoderConfig_;
    int state_;
};

class RmtBase
{
public:
    RmtBase();
    ~RmtBase();
    RmtBase(const RmtBase&) = delete;
    RmtBase(RmtBase&&) = delete;
    RmtBase& operator=(const RmtBase&) =delete;
    RmtBase& operator=(RmtBase&&) =delete;
    bool init(rmt_channel_handle_t handle);
    bool deInit();
    bool applyCarrier(uint32_t frequencyHz = 38000, float dutyCycle = 0.33, bool alwaysOn = false, bool polarityActiveLow = false);
    bool enable();
    bool disable();
    bool isInit() const {return binit_;}
    rmt_channel_handle_t getHandle() const {return handle_;}
private:
    bool binit_;
    rmt_channel_handle_t handle_;
    rmt_carrier_config_t txCarrierCfg_;
};
class MRmtTx : public RmtBase
{
public:
    MRmtTx() = default;
    ~MRmtTx() =default;
    MRmtTx(const MRmtTx&) = delete;
    MRmtTx(MRmtTx&&) = delete;
    MRmtTx& operator=(const MRmtTx&) =delete;
    MRmtTx& operator=(MRmtTx&&) =delete;
    bool init(gpio_num_t gpio, uint32_t resolutionHz , size_t memBlockSymbols, size_t transQueueDepth ,bool invertOut, bool withDma, bool ioLoopBack, bool ioOdMode, rmt_clock_source_t clkSrc = RMT_CLK_SRC_DEFAULT);
    bool registerTxEventCb(const rmt_tx_event_callbacks_t *cbs, void *userData);
    bool RemoveRegisterTxEventCb();
    bool transmit(rmt_encoder_t *encoder, const void *payload, size_t payloadBytes, const rmt_transmit_config_t *config);
    bool txWaitAllDone(int timeoutMs);

private:
    rmt_tx_channel_config_t txChanelCfg_;
};

class MRmtRx : public RmtBase
{
public:
    MRmtRx() = default;
    ~MRmtRx() = default;
    MRmtRx(const MRmtRx&) = delete;
    MRmtRx(MRmtRx&&) = delete;
    MRmtRx& operator=(const MRmtRx&) =delete;
    MRmtRx& operator=(MRmtRx&&) =delete;
    bool init(gpio_num_t gpio, uint32_t resolutionHz , size_t memBlockSymbols ,bool invertIn, bool withDma, bool ioLoopBack, bool ioOdMode, rmt_clock_source_t clkSrc = RMT_CLK_SRC_DEFAULT);
    bool registerRxEventCb(const rmt_rx_event_callbacks_t *cbs, void *userData);
    bool RemoveRegisterRxEventCb();
    bool receive(void *buffer, size_t bufferSize, const rmt_receive_config_t *config);
private:
    rmt_rx_channel_config_t rxChanelCfg_;
};