#pragma once
#include <iostream>
#include <string>
#include "esp_vfs_fat.h"
#include "mspi.h"

// IO14 is connected to the SD card of the board, the power control of the LED is IO pin
#define POWER_PIN  14

/**         SPI SD CARD
  */
#define MOUNT_POINT "/sdcard"

// on ESP32-S2, DMA channel must be the same as host id
#define SPI_DMA_CHAN    host.slot

// DMA channel to be used by the SPI peripheral
#ifndef SPI_DMA_CHAN
#define SPI_DMA_CHAN    1
#endif //SPI_DMA_CHAN

#define SD_PIN_NUM_MISO 13
#define SD_PIN_NUM_MOSI 11
#define SD_PIN_NUM_CLK  12
#define SD_PIN_NUM_CS   10
#define SD_SPI_HOST     SPI3_HOST
class MSdcard
{
public:
    MSdcard(const char* mountPoint = MOUNT_POINT);
    ~MSdcard();
    MSdcard(const MSdcard&) = delete;
    MSdcard(MSdcard&&) = delete;
    MSdcard& operator=(const MSdcard& ) = delete;
    MSdcard& operator=(MSdcard&& ) = delete;
    bool spiInit(int32_t mosiIoNum = SD_PIN_NUM_MOSI, int32_t misoIoNum = SD_PIN_NUM_MISO, int32_t sclkIoNum = SD_PIN_NUM_CLK, int32_t csIoNum = SD_PIN_NUM_CS, spi_host_device_t hostId = SD_SPI_HOST);
    bool mount();
    bool umount();
    bool isSdCardInit() const {return bsdcardInit_;}
    const std::string& getMountPoint() const {return mountPath_;}
    std::string getSdCardName();
    std::string getSdCardType();
    std::string getSdCardSpeed();
    std::string getSdCardSize();
private:
    bool bsdcardInit_;
    std::string mountPath_;
    sdmmc_host_t host_;
    sdspi_device_config_t slotDevCfg_;
    esp_vfs_fat_sdmmc_mount_config_t mountCfg_;
    sdmmc_card_t *card_;
    MSpiBus sdspiBus_;
};