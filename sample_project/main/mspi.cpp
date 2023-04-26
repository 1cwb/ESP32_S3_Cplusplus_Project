#include "mspi.h"

bool MSpiBus::spiBusInit(int32_t mosiIoNum, int32_t misoIoNum, int32_t sclkIoNum, spi_host_device_t hostId, spi_dma_chan_t dmaCh, int32_t maxTransfSz)
{
        memset(&busCfg_, 0, sizeof(busCfg_));
        busCfg_.mosi_io_num = mosiIoNum;
        busCfg_.miso_io_num = misoIoNum;
        busCfg_.sclk_io_num = sclkIoNum;
        busCfg_.quadwp_io_num = -1;
        busCfg_.quadhd_io_num = -1;
        busCfg_.max_transfer_sz = maxTransfSz;
        busCfg_.intr_flags = 0;
        hostId_ = hostId;
        esp_err_t err = spi_bus_initialize(hostId_, &busCfg_, dmaCh);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
}
bool MSpiBus::spiBusDeinit()
{
    esp_err_t err = spi_bus_free(hostId_);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}
bool MSpiBus::addDevice(MSpiDevice* dev)
{
    dev_ = dev;
    esp_err_t err = spi_bus_add_device(hostId_, dev->getDevCfg(), dev->getHandle());
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}
bool MSpiBus::removeDevice(MSpiDevice* dev)
{
    if(dev_ == dev)
    {
        dev_ = nullptr;
    }
    esp_err_t err = spi_bus_remove_device(*dev->getHandle());
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    return true;
}