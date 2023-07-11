#pragma once
#include <atomic>
#include <functional>

enum MEventID
{
    E_UI_EVNET_ID_UPDATE,
    E_UI_EVENT_ID_KEY_PRESSDOWN,
    E_UI_EVENT_ID_MAX
};

struct stUIEvent
{
    MEventID eventId;
    union {
        uint32_t val;
        uint8_t* data;
    };
    uint32_t dataLen;
    std::function<void()> clean;
};

enum MUIKeyID
{
    E_UI_KEY_EVNET_ID_INVALID,
    E_UI_KEY_EVNET_ID_OK,
    E_UI_KEY_EVNET_ID_BACK,
    E_UI_KEY_EVNET_ID_UP,
    E_UI_KEY_EVNET_ID_DOWN,
    E_UI_KEY_EVNET_ID_LEFT,
    E_UI_KEY_EVNET_ID_RIGHT,
    E_UI_KEY_EVNET_ID_MAX
};

struct stUIKeyEvent
{
    MUIKeyID keyVal;
    bool blongPress;
    uint32_t timerNum;
    bool brelease;
};

enum EMUITYPE
{
    E_UI_TYPE_INVALID,
    E_UI_TYPE_TEXT,
    E_UI_TYPE_ITEM,
    E_UI_TYPE_MAX
};

class MUiBase
{
public:
    using MUiButtonPressCb = std::function<void(MEventID, MUIKeyID, bool, uint32_t, bool)>;

    MUiBase(uint16_t x, uint16_t y, uint16_t width, uint16_t height, bool autoRegisterIncore = true, bool canbeFocus = true)
    :x_(x),y_(y),width_(width),height_(height),focusColor_(TFT_RED),bfocused_(false),binited_(false),bCanfocus_(canbeFocus),bupgradeBack_(false),autoRegisterIncore_(autoRegisterIncore),cb_(new MUiButtonPressCb),mid_(id++){}
    virtual ~MUiBase() 
    {
        binited_ = false;
        if(cb_)
        {
            delete cb_;
            cb_ = nullptr;
        }
    }
    //void setX(uint16_t x) {x_ = x;}
    //void setY(uint16_t y) {y_ = y;}
    uint16_t getX() const {return x_;}
    uint16_t getY() const {return y_;}
    uint16_t getWidth() const {return width_;}
    uint16_t getHeight() const {return height_;}
    bool bfocused() const {return bfocused_;}
    void setFocused(bool foc) {bfocused_ = foc;}
    void setCanbefocus(bool canbeFocus) {bCanfocus_ = canbeFocus;}
    bool canBefocused() const {return bCanfocus_;}
    bool bInited() const {return binited_;}
    bool needUpdateBackGround()
    {
        bool update = bupgradeBack_;
        bupgradeBack_ = false;
        return update;
    }
    uint32_t getid() const
    {
        return mid_;
    }
    //virtual const uint8_t* getUi() const = 0;
    //virtual uint32_t getUiDataLen() const = 0;
    virtual void updateData() = 0;
    virtual void onFocus() = 0;
    virtual void drawFocus() = 0;
    void pressDown(MEventID id, MUIKeyID key, bool blongPress, uint32_t timerNum, bool brelease)
    {
        if(cb_ && *cb_)
        {
            (*cb_)(id, key, blongPress, timerNum, brelease);
        }
    }
    void registerOnPressDown(const MUiButtonPressCb& cb)
    {
        if(cb_)
        {
            *cb_ = std::move(cb);
        }
    }
    void unregisterOnPressDown()
    {
        if(cb_)
        {
            *cb_ = MUiButtonPressCb();
        }
    }
    EMUITYPE getType()
    {
        return type_;
    }
    void setFocusColor(uint16_t color)
    {
        focusColor_ = color;
    }
protected:
    void setType(EMUITYPE type)
    {
        type_ = type;
    }
protected:
    uint16_t x_;
    uint16_t y_;
    uint16_t width_;
    uint16_t height_;
    uint16_t focusColor_;
    bool bfocused_;
    bool binited_;
    bool bCanfocus_;
    bool bupgradeBack_;
    bool autoRegisterIncore_;
    MUiButtonPressCb* cb_;
    EMUITYPE type_;
private:
    uint32_t mid_;
    static std::atomic<uint32_t> id;
};
std::atomic<uint32_t> MUiBase::id = 1;