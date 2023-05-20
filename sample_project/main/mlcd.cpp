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

MLcd::MLcd(uint16_t colstart, uint16_t rowstart, uint16_t initHeight, uint16_t initWidth, uint16_t width, uint16_t height) :spibus_(new MSpiBus), spidevice_(new MSpiDevice), back_(new MLed (PIN_NUM_BCKL)),pwmTimer_(new MPwmTimer),pwm_(new MPwm(back_->getPinNum(), pwmTimer_))
{
    colstart_ = colstart;
    rowstart_ = rowstart;
    initHeight_ = initHeight;
    initWidth_ = initWidth;
    width_ = width;
    height_ = height;

    spibus_->spiBusInit(PIN_NUM_MOSI, PIN_NUM_MISO, PIN_NUM_CLK, LCD_HOST, SPI_DMA_CH_AUTO, SPIFIFOSIZE * 240 * 2 + 8);
    spidevice_->init(26 * 1000 * 1000, PIN_NUM_CS, lcd_spi_pre_transfer_callback);
    spibus_->addDevice(spidevice_);
    lcdInit();
    fillScreen(TFT_BLACK);

    pwm_->channelConfig();
    pwm_->enableFade(0);
}
MLcd::~MLcd()
{
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

void MLcd::setRotation( uint8_t m)
{
    uint8_t rotation = m % 4;
    lcdCmd( ST7789_MADCTL);
    switch (rotation) {
    case 0:
        colstart_ = 52;
        rowstart_ = 40;
        width_  = initWidth_;
        height_ = initHeight_;
        lcdWriteU8( TFT_MAD_COLOR_ORDER);
        break;

    case 1:
        colstart_ = 40;
        rowstart_ = 53;
        width_  = initHeight_;
        height_ = initWidth_;
        lcdWriteU8( TFT_MAD_MX | TFT_MAD_MV | TFT_MAD_COLOR_ORDER);
        break;
    case 2:
        colstart_ = 53;
        rowstart_ = 40;
        width_  = initWidth_;
        height_ = initHeight_;
        lcdWriteU8( TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_COLOR_ORDER);
        break;
    case 3:
        colstart_ = 40;
        rowstart_ = 52;
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
    uint32_t word_tmp[16];

    while (repeats > 0) {
        uint16_t bytes_to_transfer = MIN(repeats * sizeof(uint16_t), SPIFIFOSIZE * sizeof(uint32_t));
        for (i = 0; i < (bytes_to_transfer + 3) / 4; i++) {
            word_tmp[i] = word;
        }
        spidevice_->transmit(reinterpret_cast<const uint8_t*>(word_tmp), bytes_to_transfer, nullptr, 0,(void *) 1);
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


    setAddress( x, y, x + w - 1, y + h - 1);

    lcdSendUint16R(SWAPBYTES(color), h * w);
}

void MLcd::fillScreen( uint32_t color)
{
    fillRect( 0, 0, width_, height_, color);
}

void MLcd::drawPixel(int32_t x, int32_t y, uint32_t color)
{
    setAddress(x, y, x, y);
    lcdWriteByte(color);
}

void MLcd::drawChar(uint16_t x, uint16_t y, uint8_t num, uint8_t mode, uint16_t color)
{
    uint8_t temp;
    uint8_t pos, t;
    uint16_t x0 = x;
    if (x > width_ - 16 || y > height_ - 16)return;
    num = num - ' ';
    setAddress(x, y, x + 8 - 1, y + 16 - 1);
    if (!mode) {
        for (pos = 0; pos < 16; pos++) {
            temp = asc2_1608[(uint16_t)num * 16 + pos];
            for (t = 0; t < 8; t++) {
                if (temp & 0x01)
                    lcdWriteByte(color);
                else
                    lcdWriteByte(TFT_BLACK);
                temp >>= 1;
                x++;
            }
            x = x0;
            y++;
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
        drawChar(x, y, *p, 0, color);
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