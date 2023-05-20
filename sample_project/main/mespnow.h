#pragma once
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "mwifi.h"
#include "nvs_flash.h"
#include "esp_random.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "esp_crc.h"
#include "esp_interface.h"
#include <functional>
#include <list>
#include "meventhandler.h"
enum eMespNowEventId
{
    E_ESP_NOW_EVENT_SEND = 0,
    E_ESP_NOW_EVENT_RECV,
    E_ESP_NOW_EVENT_MAX
};

struct stMespNowEventSend
{
    uint8_t macAddr[ESP_NOW_ETH_ALEN];
    esp_now_send_status_t status;
};

struct stMespNowEventRecv
{
    uint8_t macAddr[ESP_NOW_ETH_ALEN];
    uint16_t dataLen;
    uint8_t* data;
};

union uMespNowEventInfo
{
    stMespNowEventSend send;
    stMespNowEventRecv recv;
};

struct stMespNowEvent
{
    eMespNowEventId id;
    uMespNowEventInfo info;
};

class MEspNow : public MWifiBase
{
public:
    static MEspNow* getINstance()
    {
        static MEspNow mespnow_;
        return &mespnow_;
    }
    bool wifiinit(wifi_mode_t mode = WIFI_MODE_AP)
    {
        if(!setStorage(WIFI_STORAGE_RAM))
        {
            return false;
        }
        if(!setMode(mode))
        {
            return false;
        }
        mode_  = mode;
        MWifiBase::start();

#if CONFIG_ESPNOW_ENABLE_LONG_RANGE
        esp_err_t err = esp_wifi_set_protocol(mode_ == WIFI_MODE_STA ? WIFI_IF_STA : WIFI_IF_AP, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
#endif
        return true;
    }
    bool espNowInit()
    {
        esp_err_t err = esp_now_init();
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        //typedef void (*esp_now_send_cb_t)(const uint8_t *mac_addr, esp_now_send_status_t status);
        err = esp_now_register_send_cb([](const uint8_t *macAddr, esp_now_send_status_t status){
            if(macAddr == nullptr)
            {
                printf("Error: %s()%d send arg error!\n",__FUNCTION__,__LINE__);
                return;
            }
            stMsgData msg;
            stMespNowEvent* evt = static_cast<stMespNowEvent*>(malloc(sizeof(stMespNowEvent)));
            if(!evt)
            {
                printf("Error %s()%d : malloc Fail\n",__FUNCTION__,__LINE__);
                return;
            }

            msg.eventId = E_EVENT_ID_ESP_NOW;
            msg.data = reinterpret_cast<uint8_t*>(evt);
            msg.dataLen = sizeof(stMespNowEvent);
            msg.clean = [](void* data){
                if(data)
                {
                    free(data);
                }
            };
            stMespNowEventSend* psend = &evt->info.send;

            evt->id = E_ESP_NOW_EVENT_SEND;
            memcpy(psend->macAddr, macAddr, ESP_NOW_ETH_ALEN);
            psend->status = status;
            if(xQueueSend(MeventHandler::getINstance()->getQueueHandle(), &msg, ESPNOW_MAXDELAY) != pdTRUE)
            {
                printf("Error: %s()%d send queue fail!\n",__FUNCTION__,__LINE__);
            }
        });
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        //typedef void (*esp_now_recv_cb_t)(const uint8_t *mac_addr, const uint8_t *data, int data_len);
        err = esp_now_register_recv_cb([](const uint8_t *mac_addr, const uint8_t *data, int data_len){
            if(mac_addr == nullptr || data == nullptr || data_len <= 0)
            {
                printf("Error: %s()%d recv arg error!\n",__FUNCTION__,__LINE__);
                return;
            }
            stMsgData msg;
            stMespNowEvent* evt = static_cast<stMespNowEvent*>(malloc(sizeof(stMespNowEvent)));
            if(!evt)
            {
                printf("Error %s()%d : malloc Fail\n",__FUNCTION__,__LINE__);
                return;
            }

            msg.eventId = E_EVENT_ID_ESP_NOW;
            msg.data = reinterpret_cast<uint8_t*>(evt);
            msg.dataLen = sizeof(stMespNowEvent);
            msg.clean = [](void* data){
                if(data)
                {
                    stMespNowEvent* evt = static_cast<stMespNowEvent*>(data);
                    if(evt->info.recv.data)
                    {
                        free(evt->info.recv.data);
                    }
                    free(evt);
                }
            };
            
            stMespNowEventRecv* precv = &evt->info.recv;

            evt->id = E_ESP_NOW_EVENT_RECV;
            memcpy(precv->macAddr, mac_addr, ESP_NOW_ETH_ALEN);
            precv->dataLen = data_len;
            precv->data = static_cast<uint8_t*>(malloc(data_len));
            if(precv->data == nullptr)
            {
                printf("Error: %s()%d malloc fail\n",__FUNCTION__,__LINE__);
                return;
            }
            memcpy(precv->data, data, data_len);
            if(xQueueSend(MeventHandler::getINstance()->getQueueHandle(), &msg, ESPNOW_MAXDELAY) != pdTRUE)
            {
                printf("Error: %s()%d send queue fail!\n",__FUNCTION__,__LINE__);
            }
        });
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
#if CONFIG_ESP_WIFI_STA_DISCONNECTED_PM_ENABLE
        err = esp_now_set_wake_window(65535);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
#endif
        /* Set primary master key. */
        err = esp_now_set_pmk((uint8_t *)CONFIG_ESPNOW_PMK);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        if(!addPeer(broadCastMac_))
        {
            return false;
        }
        return true;
    }
    bool addPeer(const uint8_t *macAddr)
    {
        esp_err_t err = ESP_OK;
        /* Add broadcast peer information to peer list. */
        esp_now_peer_info_t *peer = static_cast<esp_now_peer_info_t*>(malloc(sizeof(esp_now_peer_info_t)));
        if(!peer)
        {
            printf("tony %s()%d Malloc Fail\n",__FUNCTION__,__LINE__);
            return false;
        }
        memset(peer, 0, sizeof(esp_now_peer_info_t));
        peer->channel = ESPNOW_CHANNEL;
        peer->ifidx =  (mode_ == WIFI_MODE_STA ? WIFI_IF_STA : WIFI_IF_AP);
        peer->encrypt = false;
        memcpy(peer->peer_addr, macAddr, ESP_NOW_ETH_ALEN);
        if(espNowIsPeerExist(macAddr))
        {
            err = esp_now_mod_peer(peer);
            if(err != ESP_OK)
            {
                printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
                free(peer);
                return false;
            }
            if(!destroyPeer(macAddr))
            {
                printf("Error: %s()%d destroyPeer fail",__FUNCTION__,__LINE__);
            }
        }
        else
        {
            err = esp_now_add_peer(peer);
            if(err != ESP_OK)
            {
                printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
                free(peer);
                return false;
            }
        }
        free(peer);
        if(!createPeer(macAddr))
        {
            printf("Error: %s()%d createPeer fail",__FUNCTION__,__LINE__);
        }
        return true;
    }
    bool espNowDelPeer(const uint8_t *macAddr)
    {
        esp_err_t err = esp_now_del_peer(macAddr);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        if(!destroyPeer(macAddr))
        {
            printf("Error: reove peer from peerlist Fail\n");
        }
        return true;
    }
    bool espNowGetPeerNum(esp_now_peer_num_t *num)
    {
        esp_err_t err = esp_now_get_peer_num(num);
        if(err != ESP_OK || num->total_num != macList_.size())
        {
            printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
            printf("Error: %s()%d num = %u, macList_.size() = %u",__FUNCTION__,__LINE__,num->total_num,macList_.size());
            return false;
        }
        return true;
    }
    bool espNowIsPeerExist(const uint8_t *peer_addr)
    {
        return esp_now_is_peer_exist(peer_addr);
    }
    
    bool espNowGetPeer(const uint8_t *peer_addr, esp_now_peer_info_t *peer)
    {
        esp_err_t err = esp_now_get_peer(peer_addr, peer);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool espNowFetchPeer(bool from_head, esp_now_peer_info_t *peer)
    {
        esp_err_t err = esp_now_fetch_peer(from_head, peer);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool espNowSetRate(wifi_phy_rate_t rate)
    {
        esp_err_t err = esp_wifi_config_espnow_rate((mode_ == WIFI_MODE_STA ? WIFI_IF_STA : WIFI_IF_AP), rate);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }

    void espNowDeinit()
    {
        esp_now_deinit();
        destroyAllPeer();
    }
    bool espSendPrivateData(const uint8_t *peer_addr, const uint8_t *data, size_t len)
    {
        esp_err_t err = esp_now_send(peer_addr, data, len);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool sendBroadCastToGetAllDevice(const uint8_t *data, size_t len)
    {
        if(len > 250 - ESP_NOW_ETH_ALEN)
        {
            len = 250 - ESP_NOW_ETH_ALEN;
        }
        size_t newLen = len + ESP_NOW_ETH_ALEN;
        uint8_t* buffer = static_cast<uint8_t*>(malloc(newLen));
        if(buffer == nullptr)
        {
            printf("Error: malloc fail\n");
            return false;
        }
        memset(buffer, 0, newLen);
        memcpy(buffer, broadCastMac_, ESP_NOW_ETH_ALEN);
        if(data != nullptr && len > 0)
        {
            memcpy(&buffer[ESP_NOW_ETH_ALEN] , data, len);
        }
        if(!espSendPrivateData(broadCastMac_, buffer, newLen))
        {
            printf("Error : send Broad Cast Info Fail\n");
            free(buffer);
            return false;
        }
        free(buffer);
        return true;
    }
    static bool isBroadCast(const uint8_t* data, uint8_t len)
    {
        if(len < ESP_NOW_ETH_ALEN)
        {
            return false;
        }
        if(data)
        {
            return (memcmp(broadCastMac_, data, ESP_NOW_ETH_ALEN) == 0);
        }
        return false;
    }
private:
    MEspNow() : CONFIG_ESPNOW_PMK("pmk1234567890123"), ESPNOW_QUEUE_SIZE(5)
    {

    }
    ~MEspNow()
    {

    }
    bool createPeer(const uint8_t *peer_addr)
    {
        uint8_t *peerAddr = static_cast<uint8_t*>(malloc(ESP_NOW_ETH_ALEN));
        if(!peerAddr)
        {
            printf("Error : %s()%d Malloc fail !\n",__FUNCTION__,__LINE__);
            return false;
        }
        memcpy(peerAddr, peer_addr, ESP_NOW_ETH_ALEN);
        macList_.emplace_back(peerAddr);
        return true;
    }
    bool destroyPeer(const uint8_t *peer_addr)
    {
        for(auto it = macList_.begin(); it != macList_.end();)
        {
            if(*it)
            {
                //printf("tony (*it)->peer_addr = "MACSTR", peerAddr = "MACSTR"\n",MAC2STR((*it)),MAC2STR(peer_addr));
                if(memcmp(*it, peer_addr, ESP_NOW_ETH_ALEN) == 0)
                {
                    free(*it);
                    it = macList_.erase(it);
                    return true;
                }
                else
                {
                    it ++;
                }
            }
            else
            {
                it = macList_.erase(it);
            }
        }
        return false;
    }
    bool destroyAllPeer()
    {
        for(auto it = macList_.begin(); it != macList_.end();)
        {
            if(*it)
            {
                //printf("tony (*it)->peer_addr = "MACSTR", peerAddr = "MACSTR"\n",MAC2STR((*it)),MAC2STR(peer_addr));
                free(*it);
                it = macList_.erase(it);
            }
            else
            {
                it = macList_.erase(it);
            }
        }
        return true;
    }
private:
    const char* CONFIG_ESPNOW_PMK;
    const uint8_t ESPNOW_QUEUE_SIZE;

    static const uint8_t broadCastMac_[ESP_NOW_ETH_ALEN];
    static const uint32_t ESPNOW_MAXDELAY = 500;
    static const uint8_t ESPNOW_CHANNEL = 1;
    static const uint8_t ESPNOW_ID = 1;

    wifi_mode_t mode_;
    std::list<uint8_t *> macList_;
};
const uint8_t MEspNow::broadCastMac_[ESP_NOW_ETH_ALEN] = {0xff,0xff,0xff,0xff,0xff,0xff};

class MespNowDataParse : public eventClient
{
    using EventSendCallBack = std::function<void(stMespNowEventSend*,bool)>;
    using EventRecvCallBack = std::function<void(stMespNowEventRecv*,bool)>;
    using EventButtonPressCb = std::function<void(uint32_t, uint32_t, uint32_t, bool, uint32_t)>;
public:
    MespNowDataParse() : sendCb_(new EventSendCallBack),
                         recvCb_(new EventRecvCallBack),
                         keyCb_(new EventButtonPressCb)
    {
        MeventHandler::getINstance()->registerClient(this);
    }
    virtual ~MespNowDataParse()
    {
        if(sendCb_)
        {
            delete sendCb_;
        }
        if(recvCb_)
        {
            delete recvCb_;
        }
        if(keyCb_)
        {
            delete keyCb_;
        }
        MeventHandler::getINstance()->unregisterClient(this);
    }
    virtual void onEvent(uint32_t eventId, uint8_t* data ,uint32_t dataLen)
    {
        if(!data)
        {
            printf("Error: %s()%d get nullptr\n",__FUNCTION__,__LINE__);
            return;
        }

        if (eventId & E_EVENT_ID_BUTTON)
        {
            if(keyCb_ && *keyCb_)
            {
                uint32_t buttonNum = 0;
                bool blongPress = 0;
                uint32_t timernum = 0;
                stButtonInfo::parseBttonInfo(reinterpret_cast<uint32_t>(data), &buttonNum, &blongPress, &timernum);
                (*keyCb_)(eventId & E_EVENT_ID_BUTTON, buttonNum, dataLen, blongPress, timernum);
            }
        }
        if(eventId & E_EVENT_ID_ESP_NOW)
        {
            uint8_t* pdataTemp = nullptr;
            bool bisBroadCast = false;
            stMespNowEvent* evt = reinterpret_cast<stMespNowEvent*>(data);
            stMespNowEvent evtTemp;
            memcpy(&evtTemp, evt, sizeof(stMespNowEvent));
            if(!evt)
            {
                printf("Error: %s()%d get nullptr\n",__FUNCTION__,__LINE__);
                return;
            }
            switch(evt->id)
            {
                case E_ESP_NOW_EVENT_SEND:
                {
                    if(sendCb_ && *sendCb_)
                    {
                        bisBroadCast = MEspNow::isBroadCast(evt->info.send.macAddr, sizeof(evt->info.send.macAddr));
                        (*sendCb_)(&evt->info.send, bisBroadCast);
                    }
                    break;
                }
                case E_ESP_NOW_EVENT_RECV:
                {
                    pdataTemp = evt->info.recv.data;
                    if(!pdataTemp)
                    {
                        printf("Error: %s()%d get nullptr\n",__FUNCTION__,__LINE__);
                        return;
                    }
                    if(recvCb_ && *recvCb_)
                    {
                        bisBroadCast = MEspNow::isBroadCast(evt->info.recv.data, evt->info.recv.dataLen);
                        if(bisBroadCast)
                        {
                            evtTemp.info.recv.data += ESP_NOW_ETH_ALEN;
                            evtTemp.info.recv.dataLen -= ESP_NOW_ETH_ALEN;
                        }
                        (*recvCb_)(&evtTemp.info.recv, bisBroadCast);
                    }
                    break;
                }
                default:
                break;
            }
        }
    }
    void setDataParseSendCb(const EventSendCallBack&& sendCb)
    {
        *sendCb_ = sendCb;
    }
    void setDataParseRecvCb(const EventRecvCallBack&& recvCb)
    {
        *recvCb_ = recvCb;
    }
    void setButtonPressCb(const EventButtonPressCb&& keycb)
    {
        *keyCb_ = keycb;
    }
private:
    EventSendCallBack* sendCb_;
    EventRecvCallBack* recvCb_;
    EventButtonPressCb* keyCb_;
};