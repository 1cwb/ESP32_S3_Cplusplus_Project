#pragma once

#include <iostream>
#include <list>
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
#include <mutex>

#define E_EVENT_ID_INVALID  0X00
#define E_EVENT_ID_KEY      BIT(0)
#define E_EVENT_ID_BUTTON   BIT(1)
#define E_EVENT_ID_ESP_NOW  BIT(2)
#define E_EVENT_ID_ESP_WIFI BIT(3)
#define E_EVENT_ID_ENCODER  BIT(4)

struct stMsgData
{
    uint32_t eventId;
    union {
        uint32_t val;
        uint8_t* data;
    };
    uint32_t dataLen;
    std::function<void(void*)> clean;
};

class eventClient
{
public:
    eventClient() : interestedEvent(E_EVENT_ID_INVALID)
    {

    }
    virtual ~eventClient()
    {
        diableAllEvent();
    }
    void enableEvent(const uint32_t eintersted)
    {
        interestedEvent |= eintersted;
    }
    void disableEvent(const uint32_t eintersted)
    {
        interestedEvent &= ~eintersted;
    }
    void diableAllEvent()
    {
        interestedEvent = E_EVENT_ID_INVALID;
    }
    uint32_t getEvent() const
    {
        return interestedEvent;
    }
    virtual void onEvent(uint32_t eventId, uint8_t* data ,uint32_t dataLen) = 0;
private:
    uint32_t interestedEvent;
};

class MeventHandler
{
public:
    static MeventHandler* getINstance()
    {
        static MeventHandler eventHandler;
        return &eventHandler;
    }
    QueueHandle_t getQueueHandle() const
    {
        return mespnowQueue_;
    }
    bool registerClient(eventClient*client)
    {
        if(!client)
        {
            return false;
        }
        std::lock_guard<std::mutex> lock(lock_);
        for(auto& c : eventClientList_)
        {
            if(client == c)
            {
                printf("Warning : The client has been registered in eventHandler\n");
                return true;
            }
        }
        eventClientList_.emplace_back(client);
        return true;
    }
    void unregisterClient(eventClient*client)
    {
        if(client)
        {
            client->diableAllEvent();
            std::lock_guard<std::mutex> lock(lock_);
            for(auto it = eventClientList_.begin(); it != eventClientList_.end();)
            {
                if((*it) == client)
                {
                    it = eventClientList_.erase(it);
                }
                else
                {
                    it ++;
                }
            }
        }
    }
    void onHandler()
    {
        stMsgData msg;
        printf("eventHandler Run...\n");
        while(true)
        {
            if(xQueueReceive(mespnowQueue_, &msg, portMAX_DELAY) == pdTRUE)
            {
                std::lock_guard<std::mutex> lock(lock_);
                for(auto& c : eventClientList_)
                {
                    if(c->getEvent() & msg.eventId)
                    {
                        c->onEvent(c->getEvent() & msg.eventId, msg.data, msg.dataLen);
                    }
                }
                if(msg.clean)
                {
                    msg.clean(msg.data);
                }
            }
        }
    }
private:
    MeventHandler()
    {
        mespnowQueue_ = xQueueCreate(ESPNOW_QUEUE_SIZE, sizeof(stMsgData));
        if (mespnowQueue_ == nullptr)
        {
            printf("Error: Create mutex fail!\n");
        }
        static std::thread evenhandlerTh_(bind(&MeventHandler::onHandler, this));
        evenhandlerTh_.detach();
    }
    ~MeventHandler()
    {
        vSemaphoreDelete(mespnowQueue_);
    }
private:
    static const int ESPNOW_QUEUE_SIZE = 50;
    QueueHandle_t mespnowQueue_;
    list<eventClient*> eventClientList_;
    std::mutex lock_;
};
