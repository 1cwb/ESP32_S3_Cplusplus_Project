#include "msdcard.h"

MSdcard::MSdcard(const char* mountPoint) : bsdcardInit_(false), mountPath_(mountPoint)
{
    sdmmc_host_t hostTemp = SDSPI_HOST_DEFAULT();
    sdspi_device_config_t slotDevCfgTemp = SDSPI_DEVICE_CONFIG_DEFAULT();
    host_ = hostTemp;
    slotDevCfg_ = slotDevCfgTemp;
}
MSdcard::~MSdcard()
{
    if(isSdCardInit())
    {
        umount();
    }
    sdspiBus_.spiBusDeinit();
}
bool MSdcard::spiInit(int32_t mosiIoNum, int32_t misoIoNum, int32_t sclkIoNum, int32_t csIoNum, spi_host_device_t hostId)
{
    if(!sdspiBus_.spiBusInit(mosiIoNum, misoIoNum, sclkIoNum, hostId, SPI_DMA_CH_AUTO, 4096))
    {
        return false;
    }
    host_.slot = static_cast<int>(sdspiBus_.getHostId());

    slotDevCfg_.gpio_cs = static_cast<gpio_num_t>(csIoNum);
    slotDevCfg_.host_id = hostId;

    mountCfg_.format_if_mount_failed = true;
    mountCfg_.max_files = 5;
    mountCfg_.allocation_unit_size = 16 * 1024;
    return true;
}
bool MSdcard::mount()
{
    esp_err_t err = esp_vfs_fat_sdspi_mount(mountPath_.c_str(), &host_, &slotDevCfg_, &mountCfg_, &card_);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    bsdcardInit_ = true;
    return true;
}
bool MSdcard::umount()
{
    esp_err_t err = esp_vfs_fat_sdcard_unmount(mountPath_.c_str(), card_);
    if(err != ESP_OK)
    {
        printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
        return false;
    }
    bsdcardInit_ = true;
    return true;
}
std::string MSdcard::getSdCardName() {return std::string(card_->cid.name);}
std::string MSdcard::getSdCardType()
{
    if(card_->is_sdio)
    {
        return std::string("SDIO");
    }
    else if(card_->is_mmc)
    {
        return std::string("MMC");
    }
    else
    {
        std::string type = (card_->ocr & (1 << 30)) ? "SDHC/SDXC" : "SDSC";
        return type;
    }
}
std::string MSdcard::getSdCardSpeed()
{
    char buff[16];
    if (card_->max_freq_khz < 1000)
    {
        sprintf(buff, "%d kHz", card_->max_freq_khz);
        return std::string(buff);
    }
    else
    {
        sprintf(buff, "%d MHz%s", card_->max_freq_khz / 1000, card_->is_ddr ? ", DDR" : "");
        return std::string(buff);
    }
}
std::string MSdcard::getSdCardSize()
{
    std::string buff;
    buff.resize(16);
    sprintf(buff.data(), "%lluMB", ((uint64_t) card_->csd.capacity) * card_->csd.sector_size / (1024 * 1024));
    return buff;
}