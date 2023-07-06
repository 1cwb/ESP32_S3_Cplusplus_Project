#pragma once
#include "meventhandler.h"
#include "muicore.h"

struct stKeyVal
{
    uint16_t keyEnter;
    uint16_t keyBack;
    uint16_t keyLeft;
    uint16_t keyRight;
    uint16_t keyUp;
    uint16_t keyDown;
};

class keyDriver : public eventClient
{
public:
    static keyDriver* getInstance()
    {
        static keyDriver mkey;
        return &mkey;
    }
    void remappingKey(const stKeyVal* key)
    {
        memcpy(key_, key, sizeof(stKeyVal));
    }
private:
    keyDriver() : key_(nullptr)
    {
        key_ = new stKeyVal;
        if(!key_)
        {
            return;
        }
        memset(key_, E_UI_KEY_EVNET_ID_INVALID, sizeof(stKeyVal));
        enableEvent(E_EVENT_ID_BUTTON);
        MeventHandler::getINstance()->registerClient(this);
    }
    ~keyDriver()
    {
        disableEvent(E_EVENT_ID_BUTTON);
        MeventHandler::getINstance()->unregisterClient(this);
        if(key_)
        {
            delete key_;
        }
    }
    MUIKeyID getKeyEvent(uint32_t buttonNum)
    {
        if(!key_)
        {
            return E_UI_KEY_EVNET_ID_INVALID;
        }
        MUIKeyID keyVal = E_UI_KEY_EVNET_ID_INVALID;
        if(buttonNum == key_->keyEnter)
        {
            keyVal = E_UI_KEY_EVNET_ID_OK;
        }
        else if(buttonNum == key_->keyBack)
        {
            keyVal = E_UI_KEY_EVNET_ID_BACK;
        }
        else if(buttonNum == key_->keyLeft)
        {
            keyVal = E_UI_KEY_EVNET_ID_LEFT;
        }
        else if(buttonNum == key_->keyRight)
        {
            keyVal = E_UI_KEY_EVNET_ID_RIGHT;
        }
        else if(buttonNum == key_->keyUp)
        {
            keyVal = E_UI_KEY_EVNET_ID_UP;
        }
        else if(buttonNum == key_->keyDown)
        {
            keyVal = E_UI_KEY_EVNET_ID_DOWN;
        }
        return keyVal;
    }
    virtual void onEvent(uint32_t eventId, uint8_t* data ,uint32_t dataLen)
    {
        if(!data)
        {
            return;
        }
        if(eventId & E_EVENT_ID_BUTTON)
        {
            stButtonInfo* btinfo = reinterpret_cast<stButtonInfo*>(data);
            MUicore::getInstance()->updateUiNotify(getKeyEvent(btinfo->gpioNum), btinfo->blongPress, btinfo->bdoubleClick, btinfo->timer, btinfo->bbuttonRelease);
        }
    }
private:
    stKeyVal* key_;
};