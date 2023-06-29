#pragma once
#include <atomic>
#include <functional>

enum EMUITYPE
{
    E_UI_TYPE_INVALID,
    E_UI_TYPE_TEXT,
    E_UI_TYPE_BUTTON,
    E_UI_TYPE_MAX
};

class MUiBase
{
public:
    using MUiButtonPressCb = std::function<void(uint32_t, uint32_t, uint32_t, bool, uint32_t, bool)>;

    MUiBase(uint16_t x, uint16_t y, uint16_t width, uint16_t height, bool canbeFocus = true)
    :x_(x),y_(y),width_(width),height_(height),bfocused_(false),binited_(false),bCanfocus_(canbeFocus),bupgradeBack_(false),cb_(new MUiButtonPressCb),mid_(id++){}
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
    virtual void pressDown(uint32_t id, uint32_t buttonNum, uint32_t len, bool blongPress, uint32_t timerNum, bool brelease) = 0;
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
    bool bfocused_;
    bool binited_;
    bool bCanfocus_;
    bool bupgradeBack_;
    MUiButtonPressCb* cb_;
    EMUITYPE type_;
private:
    uint32_t mid_;
    static std::atomic<uint32_t> id;
};
std::atomic<uint32_t> MUiBase::id = 1;