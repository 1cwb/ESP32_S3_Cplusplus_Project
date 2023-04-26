#pragma once

#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_now.h"
#include "esp_crc.h"
#include "esp_random.h"
class MWifiBase
{
public:
    MWifiBase();
    ~MWifiBase();
    MWifiBase(const MWifiBase&) = delete;
    MWifiBase(MWifiBase&&) = delete;
    MWifiBase& operator=(const MWifiBase&) =delete;
    MWifiBase& operator=(MWifiBase&&) =delete;
    static bool init();
    static bool deinit();
    static bool start();

    static bool stop();
    static bool bInited() {return initStatus_ == (ESP_WIFI_NETIF_INIT_SUCCESS | ESP_WIFI_EVENTLOOP_INIT_SUCCESS | ESP_WIFI_INIT_SUCCESS);}
    static bool bStarted() {return initStatus_ & ESP_WIFI_NETIF_STARTED;}
    bool setStorage(wifi_storage_t storage);
    bool setMode(wifi_mode_t mode);
protected:
    bool getMac(wifi_interface_t ifx, uint8_t mac[6]);
    bool setMac(wifi_interface_t ifx, const uint8_t mac[6]);
protected:
    static const uint8_t ESP_WIFI_INIT_SUCCESS = BIT(0);
    static const uint8_t ESP_WIFI_EVENTLOOP_INIT_SUCCESS = BIT(1);
    static const uint8_t ESP_WIFI_NETIF_INIT_SUCCESS = BIT(2);
    static const uint8_t ESP_WIFI_NETIF_STARTED = BIT(3);
private:
    static uint8_t initStatus_;
    static wifi_init_config_t initCfg_;
};

class MWifiStation : public MWifiBase
{
public:
    static MWifiStation* getINstance();
    bool init(bool onlyStaMode = true);
    bool deinit();
    bool wifiScanStart(const wifi_scan_config_t *config = nullptr, bool block = true);
    bool wifiScanStop();
    bool wifiScanGetApRecords(wifi_ap_record_t** apInfo);
    bool wifiGetScanApNum(uint16_t* apNum);
    bool getMac(uint8_t mac[6])
    {
        return MWifiBase::getMac(WIFI_IF_STA, mac);
    }
    bool setMac(const uint8_t mac[6])
    {
        return MWifiBase::setMac(WIFI_IF_STA, mac);
    }
private:

    MWifiStation() : staNetif_(nullptr)
    {
        memset(apInfo_, 0, sizeof(apInfo_));
    }
    ~MWifiStation()
    {

    }
private:
    static const uint16_t DEFAULT_SCAN_LIST_SIZE = 16;

    esp_netif_t *staNetif_;
    wifi_ap_record_t apInfo_[DEFAULT_SCAN_LIST_SIZE];
};

class MWifiAP : public MWifiBase
{
public:
    static MWifiAP* getINstance();
    bool init(bool onlyApMode = true, const char* ssid = "hellowworld", const char* passwd = "cwb1994228", const uint8_t channel = 1/*1-13*/, const wifi_auth_mode_t authmode = WIFI_AUTH_WPA2_PSK, const bool ssidHidden = false, const uint8_t maxConnection = 4, const wifi_cipher_type_t pairwiseCipher = WIFI_CIPHER_TYPE_TKIP, const bool ftmResponder = false, const bool pmfCapAble = false, const bool pmfRequired = true);
    bool startAp();
    bool stopAp();
    bool deinit();
    bool wifiEventRegister(esp_event_handler_t event_handler);
    bool wifiEventUnregister();
    bool getMac(uint8_t mac[6])
    {
        return MWifiBase::getMac(WIFI_IF_AP, mac);
    }
    bool setMac(const uint8_t mac[6])
    {
        return MWifiBase::setMac(WIFI_IF_AP, mac);
    }
private:
    MWifiAP():apNetif_(nullptr)
    {
        
    }
    ~MWifiAP(){}
private:
    esp_netif_t* apNetif_;
    wifi_config_t apConfig_;
};

class MWifiAPSTA
{
public:
    static MWifiAPSTA* getINstance()
    {
        static MWifiAPSTA wifiapsta;
        return &wifiapsta;
    }
    MWifiStation* getSTA()
    {
        return msta_;
    }
    MWifiAP* getAP()
    {
        return ap_;
    }
    bool init()
    {
        if(!msta_->init(false))
        {
            return false;
        }
        if(!ap_->init(false))
        {
            return false;
        }
        ap_->start();
        return true;
    }

private:
    MWifiAPSTA():msta_(MWifiStation::getINstance()),ap_(MWifiAP::getINstance()){}
    ~MWifiAPSTA(){}
private:
    MWifiStation* msta_;
    MWifiAP* ap_;
};
