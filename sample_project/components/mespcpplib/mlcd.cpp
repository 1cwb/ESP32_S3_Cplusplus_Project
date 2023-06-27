#include "mlcd.h"
#include "font.h"

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

void lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc = (int)t->user;
    gpio_set_level(PIN_NUM_DC, dc);
}

MLcd::MLcd(uint16_t colstart, uint16_t rowstart, uint16_t initHeight, uint16_t initWidth, uint16_t width, uint16_t height,  uint8_t pixelOfBit) 
:oricolstart_(colstart),orirowstart_(rowstart),colstart_(colstart),rowstart_(rowstart),initHeight_(initHeight),initWidth_(initWidth),width_(width),height_(height),pixelOfBit_(pixelOfBit),lcdRamSize_(initHeight_ * initWidth_ * pixelOfBit_),maxTransFerBytes_(SPIFIFOSIZE * initWidth_ * pixelOfBit_),
spibus_(new MSpiBus), spidevice_(new MSpiDevice), back_(new MLed (PIN_NUM_BCKL)),pwmTimer_(new MPwmTimer),pwm_(new MPwm(back_->getPinNum(), pwmTimer_)),lcdRam_(new uint8_t[lcdRamSize_])
{
    memset(lcdRam_, 0, lcdRamSize_);
    spibus_->spiBusInit(PIN_NUM_MOSI, PIN_NUM_MISO, PIN_NUM_CLK, LCD_HOST, SPI_DMA_CH_AUTO, SPIFIFOSIZE * initWidth * pixelOfBit + 8);
    spidevice_->init(80 * 1000 * 1000, PIN_NUM_CS, lcd_spi_pre_transfer_callback);
    spibus_->addDevice(spidevice_);
    lcdInit();
    fillScreen(TFT_BLACK);

    pwm_->channelConfig();
    pwm_->enableFade(0);
}
MLcd::~MLcd()
{
    spidevice_->deinit();
    spibus_->removeDevice(spidevice_);
    spibus_->spiBusDeinit();
    if(pwm_)
    {
        delete pwm_;
    }
    if(pwmTimer_)
    {
        delete pwmTimer_;
    }
    if(back_)
    {
        delete back_;
    }
    if(spidevice_)
    {
        delete spidevice_;
    }
    if(spibus_)
    {
        delete spibus_;
    }
    if(lcdRam_)
    {
        delete [] lcdRam_;
    }
}
void MLcd::lcdInit()
{
    //Initialize non-SPI GPIOs
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
    //gpio_set_direction(PIN_NUM_BCKL, GPIO_MODE_OUTPUT);

    //Reset the display
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);


    lcdCmd( ST7789_SLPOUT);                // Sleep out
    vTaskDelay(120 / portTICK_PERIOD_MS);

    lcdCmd( ST7789_NORON);                 // Normal display mode on

    //------------------------------display and color format setting--------------------------------//
    lcdCmd( ST7789_MADCTL);
    lcdWriteU8( TFT_MAD_RGB);

    // JLX240 display datasheet
    lcdCmd( 0xB6);
    lcdWriteU8( 0x0A);
    lcdWriteU8( 0x82);

    lcdCmd( ST7789_COLMOD);
    lcdWriteU8( 0x55);
    vTaskDelay(10 / portTICK_PERIOD_MS);

    //--------------------------------ST7789V Frame rate setting----------------------------------//
    lcdCmd( ST7789_PORCTRL);
    lcdWriteU8( 0x0c);
    lcdWriteU8( 0x0c);
    lcdWriteU8( 0x00);
    lcdWriteU8( 0x33);
    lcdWriteU8( 0x33);

    lcdCmd( ST7789_GCTRL);                 // Voltages: VGH / VGL
    lcdWriteU8( 0x35);

    //---------------------------------ST7789V Power setting--------------------------------------//
    lcdCmd( ST7789_VCOMS);
    lcdWriteU8( 0x28);                    // JLX240 display datasheet

    lcdCmd( ST7789_LCMCTRL);
    lcdWriteU8( 0x0C);

    lcdCmd( ST7789_VDVVRHEN);
    lcdWriteU8( 0x01);
    lcdWriteU8( 0xFF);

    lcdCmd( ST7789_VRHS);                  // voltage VRHS
    lcdWriteU8( 0x10);

    lcdCmd( ST7789_VDVSET);
    lcdWriteU8( 0x20);

    lcdCmd( ST7789_FRCTR2);
    lcdWriteU8( 0x0f);

    lcdCmd( ST7789_PWCTRL1);
    lcdWriteU8( 0xa4);
    lcdWriteU8( 0xa1);

    //--------------------------------ST7789V gamma setting---------------------------------------//
    lcdCmd( ST7789_PVGAMCTRL);
    lcdWriteU8( 0xd0);
    lcdWriteU8( 0x00);
    lcdWriteU8( 0x02);
    lcdWriteU8( 0x07);
    lcdWriteU8( 0x0a);
    lcdWriteU8( 0x28);
    lcdWriteU8( 0x32);
    lcdWriteU8( 0x44);
    lcdWriteU8( 0x42);
    lcdWriteU8( 0x06);
    lcdWriteU8( 0x0e);
    lcdWriteU8( 0x12);
    lcdWriteU8( 0x14);
    lcdWriteU8( 0x17);

    lcdCmd( ST7789_NVGAMCTRL);
    lcdWriteU8( 0xd0);
    lcdWriteU8( 0x00);
    lcdWriteU8( 0x02);
    lcdWriteU8( 0x07);
    lcdWriteU8( 0x0a);
    lcdWriteU8( 0x28);
    lcdWriteU8( 0x31);
    lcdWriteU8( 0x54);
    lcdWriteU8( 0x47);
    lcdWriteU8( 0x0e);
    lcdWriteU8( 0x1c);
    lcdWriteU8( 0x17);
    lcdWriteU8( 0x1b);
    lcdWriteU8( 0x1e);

    lcdCmd(ST7789_INVON);

    lcdCmd( ST7789_DISPON);   //Display on
    vTaskDelay(120 / portTICK_PERIOD_MS);

    ///Enable backlight
    gpio_set_level(PIN_NUM_BCKL, 1);
}

void MLcd::lcdWriteByte( const uint16_t data)
{
    uint8_t val;
    val = data >> 8 ;
    lcdData( &val, 1);
    val = data;
    lcdData( &val, 1);
}
void MLcd::setAddress( uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    lcdCmd( ST7789_CASET);
    lcdWriteByte( x1 + colstart_);
    lcdWriteByte( x2 + colstart_);
    lcdCmd( ST7789_RASET);
    lcdWriteByte( y1 + rowstart_);
    lcdWriteByte( y2 + rowstart_);
    lcdCmd( ST7789_RAMWR);
}

void MLcd::setRotation( eRotation m)
{
    uint8_t rotation = m % 4;
    lcdCmd( ST7789_MADCTL);
    switch (rotation) {
    case E_ROTATION_0:
        colstart_ = oricolstart_;
        rowstart_ = orirowstart_;
        width_  = initWidth_;
        height_ = initHeight_;
        lcdWriteU8( TFT_MAD_COLOR_ORDER);
        break;

    case E_ROTATION_90:
        colstart_ = orirowstart_;
        rowstart_ = oricolstart_;
        width_  = initHeight_;
        height_ = initWidth_;
        lcdWriteU8( TFT_MAD_MX | TFT_MAD_MV | TFT_MAD_COLOR_ORDER);
        break;
    case E_ROTATION_180:
        colstart_ = oricolstart_;
        rowstart_ = orirowstart_;
        width_  = initWidth_;
        height_ = initHeight_;
        lcdWriteU8( TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_COLOR_ORDER);
        break;
    case E_ROTATION_270:
        colstart_ = orirowstart_;
        rowstart_ = oricolstart_;
        width_  = initHeight_;
        height_ = initWidth_;
        lcdWriteU8( TFT_MAD_MV | TFT_MAD_MY | TFT_MAD_COLOR_ORDER);
        break;
    }
}



void MLcd::lcdSendUint16R(const uint16_t data, int32_t repeats)
{
    uint32_t i;
    uint32_t word = data << 16 | data;
    uint32_t word_tmp[SPIFIFOSIZE] = {0};

    while (repeats > 0) {
        uint16_t bytes_to_transfer = MIN(repeats * sizeof(uint16_t), SPIFIFOSIZE * sizeof(uint32_t));
        for (i = 0; i < (bytes_to_transfer + 3) / 4; i++) {
            word_tmp[i] = word;
        }
        spidevice_->transferBytes(reinterpret_cast<const uint8_t*>(word_tmp), nullptr, bytes_to_transfer, (void *) 1); //transmit(reinterpret_cast<const uint8_t*>(word_tmp), bytes_to_transfer, nullptr, 0,(void *) 1);
        repeats -= bytes_to_transfer / 2;
    }
}


void MLcd::fillRect( int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color)
{
    // Clipping
    if ((x >= width_) || (y >= height_)) return;

    if (x < 0) {
        w += x;
        x = 0;
    }
    if (y < 0) {
        h += y;
        y = 0;
    }

    if ((x + w) > width_)  w = width_  - x;
    if ((y + h) > height_) h = height_ - y;

    if ((w < 1) || (h < 1)) return;
    std::lock_guard<std::mutex> lock(mutex_);
    for(uint32_t j = 0; j < h; j++)
    {
        for(uint32_t i = 0; i < w; i++)
        {
            drawPixel(x + i, y + j, color);
        }
    }
}

void MLcd::fillScreen( uint32_t color)
{
    fillRect( 0, 0, width_, height_, color);
}

void MLcd::drawPixel(int32_t x, int32_t y, uint32_t color)
{
    uint8_t val;
    uint32_t index = (x + (y*width_))*2;
    val = color >> 8 ;
    lcdRam_[index] = val;

    val = color;
    lcdRam_[index + 1] = val;
}

void MLcd::drawChar(uint16_t x, uint16_t y, uint8_t num, uint8_t mode, uint16_t color, uint16_t backColor)
{
    uint8_t temp;
    uint8_t pos, t;
    if(color == backColor)
    {
        backColor = ~color;
    }
    if (x > width_ - 16 || y > height_ - 16)return;
    num = num - ' ';
    std::lock_guard<std::mutex> lock(mutex_);
    if (!mode) {
        for (pos = 0; pos < 16; pos++) {
            temp = asc2_1608[(uint16_t)num * 16 + pos];
            for (t = 0; t < 8; t++) {
                if (temp & 0x01)
                    drawPixel(x + t, y + pos, color);
                else
                    drawPixel(x + t, y + pos, backColor);
                temp >>= 1;
            }
        }
    } else {
        for (pos = 0; pos < 16; pos++) {
            temp = asc2_1608[(uint16_t)num * 16 + pos];
            for (t = 0; t < 8; t++) {
                if (temp & 0x01)
                    drawPixel(x + t, y + pos, color);
                temp >>= 1;
            }
        }
    }
}

void MLcd::drawString(uint16_t x, uint16_t y, const char *p, uint16_t color, uint16_t backColor)
{
    while (*p != '\0') {
        if (x > width_ - 16) {
            x = 0;
            y += 16;
        }
        if (y > height_ - 16) {
            y = x = 0;
            fillScreen(TFT_RED);
        }
        drawChar(x, y, *p, 0, color, backColor);
        x += 8;
        p++;
    }
}

void MLcd::drawString(uint16_t x, uint16_t y, const char *p, uint16_t color)
{
    while (*p != '\0') {
        if (x > width_ - 16) {
            x = 0;
            y += 16;
        }
        if (y > height_ - 16) {
            y = x = 0;
            fillScreen(TFT_RED);
        }
        drawChar(x, y, *p, 1, color, color);
        x += 8;
        p++;
    }
}

void MLcd::setBackLight(uint8_t percent)
{
    if(percent > 100)
    {
        percent = 100;
    }
    pwm_->setDutyAndUpdate(percent * (8000 / 100));
}
void MLcd::setRamData(const uint8_t* data, uint32_t len)
{
    std::lock_guard<std::mutex> lock(mutex_);
    memcpy(lcdRam_, data, len > lcdRamSize_ ? lcdRamSize_ : len);
}

void MLcd::drawInRam(int32_t x, int32_t y, int32_t w, int32_t h, const unsigned char* picture, uint32_t len)
{
    if(!picture || len == 0)
    {
        return;
    }
    if ((x >= width_) || (y >= height_)) return;

    if (x < 0) {
        w += x;
        x = 0;
    }
    if (y < 0) {
        h += y;
        y = 0;
    }

    if ((x + w) > width_)  w = width_  - x;
    if ((y + h) > height_) h = height_ - y;

    if ((w < 1) || (h < 1)) return;
    const unsigned char* p = picture;
    uint32_t pixelWidth = w * pixelOfBit_;
    uint32_t xmap = x*pixelOfBit_;
    uint32_t xlen = getWidth()*pixelOfBit_;
    if( (x == 0) && (y == 0) && (w = getWidth()) && (h == getHeight()))
    {
        setRamData(picture, lcdRamSize_);
    }
    else
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for(uint32_t j = 0; j < h; j++)
        {
            for(uint32_t i = 0; i < pixelWidth; i++)
            {
                printf("n\n");
                lcdRam_[(uint32_t)((xmap+i) + (y+j)*xlen)] = *(p++);
                if(p >= picture + len)
                {
                    return;
                }
            }
        }
    }
}

void MLcd::reFreshFrame()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if(lcdRamSize_ > maxTransFerBytes_)
    {
        for(uint32_t i = 0; i < lcdRamSize_; i += maxTransFerBytes_)
        {
            if(lcdRamSize_ - i > maxTransFerBytes_)
            {
                lcdData(lcdRam_ + i, maxTransFerBytes_);
            }
            else
            {
                lcdData(lcdRam_ + i, lcdRamSize_ - i);
            }
        }
    }
    else
    {
        lcdData(lcdRam_, lcdRamSize_);
    }
}
void MLcd::reFreshFrame(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const uint8_t* ram, uint32_t ramlen)
{
    if(!ram)
    {
        return;
    }
    setAddress(x1, y1, x2, y2);
    if(ramlen > maxTransFerBytes_)
    {
        for(uint32_t i = 0; i < ramlen; i += maxTransFerBytes_)
        {
            if(ramlen - i > maxTransFerBytes_)
            {
                lcdData(ram + i, maxTransFerBytes_);
            }
            else
            {
                lcdData(ram + i, ramlen - i);
            }
        }
    }
    else
    {
        lcdData(ram, ramlen);
    }
}