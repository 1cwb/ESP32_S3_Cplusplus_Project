#include <stdio.h>
#include <iostream>
#include <fstream>
#include <thread>
#include "freertos/FreeRtos.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "mled.h"
#include "mpwm.h"
#include "mtimer.h"
#include "mspi.h"
#include "mlcd.h"
#include "font.h"
#include "mgpio.h"
#include "msdcard.h"
using namespace std;

#if 0
extern "C" void app_main(void)
{
    MGpio power((gpio_num_t)POWER_PIN);
    power.setLevel(1);
    MSdcard card;
    card.spiInit();
    card.mount();
    if(card.isSdCardInit())
    {
        cout << "SD NAME: " << card.getSdCardName() << endl;
        cout << "SD SIZE: " << card.getSdCardSize() << endl;
        cout << "SD Speed: " << card.getSdCardSpeed() << endl;
        cout << "SD Type: " << card.getSdCardType() << endl;
        printf("mount ok \n");
        ofstream outfile;
        outfile.open(MOUNT_POINT"/text.txt", ios::out | ios::trunc );
        if(outfile.is_open())
        {
            cout << "open ok" << endl;
            outfile.write("hellow world",strlen("hellow world"));
            outfile << "rinimabi" << endl;
            outfile.close();
        }

        ifstream infile;
        infile.open(MOUNT_POINT"/text.txt", ios::out | ios::in );
        if(outfile.is_open())
        {
            char buf[strlen("hellow world") + 1];
            infile.read(buf,strlen("hellow world"));
            infile.close();
            printf("read data: %s\n",buf);
        }
    }

    MLed back(33);
    back.ON();
    MPwmTimer pwmtimer;
    MPwm pwm(back.getPinNum(),&pwmtimer);
    pwm.channelConfig();
    pwm.enableFade(1);
    pwm.fadeEndCbRegister([](const ledc_cb_param_t *param, void *user_arg)->bool{
        if(param->event == LEDC_FADE_END_EVT)
        {
            MPwm* p = static_cast<MPwm*>(user_arg);
            if(p != nullptr)
            {
                if(p->getDuty() >= 8000)
                {
                    p->setFadeStepAndStart(0,8,5,LEDC_FADE_NO_WAIT);
                }
                else
                {
                    p->setFadeStepAndStart(8000,8,5,LEDC_FADE_NO_WAIT);
                }
            }
        }
        return true;
    },&pwm);
    pwm.setFadeStepAndStart(8000,8,5,LEDC_FADE_NO_WAIT);
    while (true)
    {
        //cout << "back.OFF()" << endl;
        //back.OFF();
        //TickType_t  timer1 = xTaskGetTickCount();
        
        //cout << "use time: " << xTaskGetTickCount() - timer1 << endl;
        //vTaskDelay(3000/portTICK_PERIOD_MS);
        //back.ON();
        //cout << "back.ON()" << endl;
        
        vTaskDelay(1000/portTICK_PERIOD_MS);
        //back.OFF();
        vTaskDelay(3000/portTICK_PERIOD_MS);
        //back.ON();
    }
    
}
#endif
#if 0
    static IRAM_ATTR bool cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx){
        MLed* pled = static_cast<MLed*>(user_ctx);
        if(pled->isOn())
        {
            pled->OFF();
        }
        else
        {
            pled->ON();
        }
        return true;
    }
extern "C" void app_main(void)
{
    MLed led(33);
    led.OFF();
    MTimer timer;
    timer.init(1000000);
    timer.enableAlarm(500000, 0, true);
    timer.registerEventCallbacks(cb, &led);
    timer.enable();
    timer.start();
    while (true)
    {
        //led.ON();
         vTaskDelay(1000/portTICK_PERIOD_MS);
                 //led.OFF();
         vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}
#endif
#if 0
QueueHandle_t gpioQueue = nullptr;
extern "C" void app_main(void)
{
    gpioQueue = xQueueCreate(10, sizeof(void*));
    MLcd lcd;
    MGpio mkey(GPIO_NUM_0 , GPIO_MODE_INPUT, GPIO_PULLUP_ENABLE);
    mkey.setIntrType();
    mkey.installIsrService();
    mkey.addIsrHandler([](void* arg){
        uint32_t ukey = reinterpret_cast<uint32_t>(arg);
        xQueueSendFromISR(gpioQueue, &ukey, nullptr);
    }, &mkey);
    mkey.intrEnable();
    lcd.setRotation(0);

    lcd.setAddress( 0, 0,  lcd.getWidth() - 1, lcd.getHeight() - 1);
    /*for (uint32_t i = 0; i < sizeof(gImage_1) ; i++) {
        lcd.lcdData( &gImage_1[i], 1);
    }*/
    lcd.setBackLight(20);

    thread test([&lcd](){
        static int m = 0;
        MGpio* pkey = nullptr;
        uint32_t add = 0;
        while(true)
        {
            xQueueReceive(gpioQueue, &add, portMAX_DELAY);
            pkey = reinterpret_cast<MGpio*>(add);
            if(pkey)
            {
                if(pkey->getLevel() == 0)
                {
                    m ++;
                }
                if(m %2 == 0)
                {
                    lcd.drawString(0,0,"hellow world1",TFT_RED);
                }
                else
                {
                    lcd.drawString(0,0,"hellow world2",TFT_BLUE);
                }
            }
        }
    });
    //lcd.drawString(0,0,"hellow world",TFT_RED);

    //gpio_pad_select_gpio((gpio_num_t)POWER_PIN);
    /* Set the GPIO as a push/pull output */
    MGpio power((gpio_num_t)POWER_PIN);
    power.setLevel(1);
    while (true)
    {
        //led.ON();
         vTaskDelay(1000/portTICK_PERIOD_MS);
                 //led.OFF();
         vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}
#endif

#if 0
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "madc.h"
#include "mrmt.h"
#include "mrgbcolor.h"
#include "mledstrip.h"
#define DEFAULT_VERF                            1100

extern "C" void app_main(void)
{
    int v = 0;
    MAdc adc1;
    adc1.oneShortUnitInit(ADC_UNIT_1, ADC_ULP_MODE_DISABLE);
    adc1.oneShortChanConfig(GPIO_NUM_1, ADC_ATTEN_DB_11, ADC_BITWIDTH_DEFAULT);
    adc1.caliFittingConfig();

    LedStrip ledstrip;
    ledstrip.init();
    ledstrip.setRGBAndUpdate(RGB_COLOR_BLACK);
    uint32_t temp = 0;
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
        if(adc1.calibrated())
        {
            adc1.caliRawToVoltage(&v);
        }
        printf("get v = %d\n",v);
        temp = (255 * v) / 3100;
        if(temp > 255) temp = 255;
        ledstrip.setRGBAndUpdate(temp,temp/2,temp/3);

        char buff[128];
        snprintf(buff, 128, "V:%u", v);
        printf("%s\n",buff);
    }
}
#endif


#if 0
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "madc.h"
#include "mrmt.h"
#include "mledstrip.h"
#include "mrgbcolor.h"
#include "madc.h"
#include "mwifi.h"
#define DEFAULT_VERF                            1100

extern "C" void app_main(void)
{
    LedStrip ledstrip;
    ledstrip.init();
    ledstrip.setRGBAndUpdate(RGB_COLOR_YELLOW);
    
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
/*
    MWifiStation* mwifi = MWifiStation::getINstance();
    mwifi->init(false);

    MWifiAP*ap = MWifiAP::getINstance();
    ap->wifiEventRegister([](void* event_handler_arg,
                                        esp_event_base_t event_base,
                                        int32_t event_id,
                                        void* event_data)
    {
            if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI("wifi", "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI("wifi", "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }

    });
    ap->init(false);

    //ap->start();
    mwifi->start();
    mwifi->wifiScanStart();
    wifi_ap_record_t* info;
    uint16_t apnum;
    mwifi->wifiScanGetApRecords(&info);
    mwifi->wifiGetScanApNum(&apnum);
    
    printf("apnum is %u\n",apnum);
    for(int i = 0; i < apnum; i++)
    {
        cout << info[i].ssid << endl;
    }
*/
    MWifiAPSTA* apsta = MWifiAPSTA::getINstance();
    apsta->getAP()->wifiEventRegister([](void* event_handler_arg,
                                        esp_event_base_t event_base,
                                        int32_t event_id,
                                        void* event_data)
    {
            if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI("wifi", "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
                 //ledstrip.setRGBAndUpdate(RGB_COLOR_GREEN);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI("wifi", "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
                 //ledstrip.setRGBAndUpdate(RGB_COLOR_RED);
    }});
    apsta->init();
    apsta->getSTA()->wifiScanStart();
    wifi_ap_record_t* info;
    uint16_t apnum;
    apsta->getSTA()->wifiScanGetApRecords(&info);
    apsta->getSTA()->wifiGetScanApNum(&apnum);
    
    printf("apnum is %u\n",apnum);
    for(int i = 0; i < apnum; i++)
    {
        cout << info[i].ssid << endl;
    }
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        //char buff[128];
        //snprintf(buff, 128, "V:%u", v);
        //printf("%s\n",buff);
    }
}
#endif

#if 0
#include "mrgbcolor.h"
#include "mespnow.h"
#include "mledstrip.h"
#include "mbutton.h"
#include <thread>
#include "meventhandler.h"
using namespace std;
struct stEspNowControlMsg
{
    uint8_t ledId;
    uint8_t ledStatus;
    uint8_t ledR;
    uint8_t ledG;
    uint8_t ledB;
} __attribute__((packed));
extern "C" void app_main(void)
{
    MeventHandler* evhand = MeventHandler::getINstance();
    MButton button(GPIO_NUM_45);
    button.registerButtonHandler([](void* pa){

    },&button);
    button.enable();
    LedStrip ledstrip;
    ledstrip.init();
    ledstrip.setRGBAndUpdate(RGB_COLOR_BLACK);
    
    espnow->registerEspNowSendCb([&](stMespNowEventSend* send, bool bBroadCast){
        //ledstrip.setRGBAndUpdate(RGB_COLOR_RED);
        printf("SEND MAC: "MACSTR"\n",MAC2STR(send->macAddr));

    });
    espnow->registerEspNowRecvCb([&](stMespNowEventRecv* recv,bool bBroadCast){
        ledstrip.setRGBAndUpdate(RGB_COLOR_BLUE);
        printf("recv MAC: "MACSTR"\n",MAC2STR(recv->macAddr));
        printf("recv datalen = %d\n",recv->dataLen);
        if(bBroadCast)
        {
            printf("recieve a broad cast\n");
            espnow->addPeer(recv->macAddr);
            espnow->sendBroadCastToGetAllDevice(NULL,0);
        }
        else
        {
            stEspNowControlMsg *leadMsg = reinterpret_cast<stEspNowControlMsg*>(recv->data);
            if(leadMsg->ledId == 1)
            {
                ledstrip.setRGBAndUpdate(leadMsg->ledR,leadMsg->ledG,leadMsg->ledB);
            }
        }
    });
    thread th1(bind(&MEspNow::EventHandler, espnow));    
    espnow->sendBroadCastToGetAllDevice(NULL,0);
    while (true)
    {
        //led.ON();
         vTaskDelay(2000/portTICK_PERIOD_MS);
                 //led.OFF();
                 //espnow->espSend(0);
         //vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}
#endif

#if 0
#include "mrgbcolor.h"
#include "mespnow.h"
#include "mledstrip.h"
#include "mbutton.h"
#include <thread>
#include "meventhandler.h"
using namespace std;

extern "C" void app_main(void)
{
    LedStrip ledstrip;
    ledstrip.init();
    ledstrip.setRGBAndUpdate(RGB_COLOR_BLACK);

    MeventHandler* evhand = MeventHandler::getINstance();
    MespNowDataParse espnowData;
    espnowData.enableEvent(E_EVENT_ID_KEY|E_EVENT_ID_ESP_NOW);

    evhand->registerClient(&espnowData);

    MButton button(GPIO_NUM_45);
    button.registerButtonHandler([](void* pa){
        BaseType_t xTaskWokenByReceive = pdFALSE;
        stMsgData keyEvent = {E_EVENT_ID_KEY, 3799, 4};
        xQueueSendFromISR(MeventHandler::getINstance()->getQueueHandle(), &keyEvent, &xTaskWokenByReceive);
    },&button);
    button.enable();

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
    MEspNow* espnow = MEspNow::getINstance();
    espnow->wifiinit();
    espnow->espNowInit();

    espnowData.setKeyPressCb([&](uint32_t id, uint32_t data, uint32_t len)
    {
        cout << "key value = " << data << endl;
        cout << "data len = " << len << endl;
        ledstrip.setRGBAndUpdate(RGB_COLOR_BLACK);
    });

    thread th1(bind(&MeventHandler::onHandler, evhand));
    while (true)
    {
         vTaskDelay(2000/portTICK_PERIOD_MS);
    }
}
#endif
#if 0
#include <thread>
#include "mledstrip.h"
#include "mrgbcolor.h"
#include "mbutton.h"
#include "meventhandler.h"
#include "mespnow.h"
#include <iostream>
#include "mpcnt.h"
#include "motor.h"
#include "mespcmd.h"
using namespace std;

#define BDC_MCPWM_TIMER_RESOLUTION_HZ 10000000 // 10MHz, 1 tick = 0.1us
#define BDC_MCPWM_FREQ_HZ             50000    // 25KHz PWM
#define BDC_MCPWM_DUTY_TICK_MAX       (BDC_MCPWM_TIMER_RESOLUTION_HZ / BDC_MCPWM_FREQ_HZ) // maximum value we can set for the duty cycle, in ticks

extern "C" void app_main(void)
{
    LedStrip ledstrip;
    ledstrip.init();
    ledstrip.setRGBAndUpdate(RGB_COLOR_BLACK);

    MespNowDataParse* espnowData = new MespNowDataParse ;
    espnowData->enableEvent(E_EVENT_ID_BUTTON|E_EVENT_ID_ESP_NOW);
    
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
    MEspNow* espnow = MEspNow::getINstance();
    espnow->wifiinit();
    espnow->espNowInit();

    MButton button(GPIO_NUM_45);
    MButton buttonBoot(GPIO_NUM_0);

    buttonBoot.setButtonPressCb([&](uint32_t id, uint32_t buttonNum, uint32_t len, bool blongPress, uint32_t timerNum, bool brelease){
            if(blongPress && timerNum >= 500 && timerNum< 510)
            {
                stBaseCmd connectCmd;
                connectCmd.setData(E_ESP_CMD_ID_CONNECT, nullptr, 0);
                espnow->sendBroadCastToGetAllDevice(reinterpret_cast<uint8_t*>(&connectCmd), sizeof(connectCmd));
            }
            //servoPwm.swSetDutyAndUpdate(MOTOR_SERVO_DEGREE_45);
            //MotorPwm.swSetDutyAndUpdate(600);
        });

    Mencoder* encoder1 = new Mencoder;
    encoder1->init(6,7);
    encoder1->addWatchPoint(-1000);
    encoder1->addWatchPoint(-500);
    encoder1->addWatchPoint(0);
    encoder1->addWatchPoint(500);
    encoder1->addWatchPoint(1000);
    encoder1->start();
    MEncoderParse* encoderParse = new MEncoderParse;
    encoderParse->setEncoderCb([&](pcnt_unit_handle_t handle, pcnt_watch_event_data_t* watchEvData){
        if(handle == encoder1->getUnit()->getPcntUnitHand())
        {
            //cout << "value = " << watchEvData->watch_point_value <<endl;
            //cout << "get value = " << encoder1->getUnit()->getCount() <<endl;
        }
    });
    uint32_t speed = (BDC_MCPWM_TIMER_RESOLUTION_HZ / BDC_MCPWM_FREQ_HZ);
    printf("speed = %lu\n",speed);
    Motor *motor = new Motor(0,BDC_MCPWM_FREQ_HZ, BDC_MCPWM_TIMER_RESOLUTION_HZ, GPIO_NUM_4,GPIO_NUM_5);
    motor->enable();
    motor->forward();
    motor->setSpeed(0);

    espnowData->setDataParseRecvCb([&](stMespNowEventRecv* recv,bool isbroadCast){
        stBaseCmd* mcmd = reinterpret_cast<stBaseCmd*>(recv->data);
        if(!isbroadCast && mcmd->eid == E_ESP_CMD_ID_CONNECT)//connect
        {
            espnow->addPeer(recv->macAddr);
            ledstrip.setRGBAndUpdate(RGB_COLOR_BLUE);
        }
        if(mcmd->eid == E_ESP_CMD_ID_SEND_STR)
        {
            char data[128] = {0};
            memcpy(data, mcmd->udata.buf, mcmd->dataLen);
            printf("recv str : %s, mcmd->dataLen = %u\n",data,mcmd->dataLen);
        }
    });

    while(true)
    {
        vTaskDelay(10000/portTICK_PERIOD_MS);
        //motor1.getUnit()->getCount(&c);
       // motor2.getUnit()->getCount(&d);
        //cout <<"c" << c << "d" << d <<  endl;
        //motor->forward();
        //vTaskDelay(10000/portTICK_PERIOD_MS);
        //motor->reverse();
    }
    delete espnowData;
    delete encoderParse;
    delete encoder1;
    delete motor;
}
#endif

#if 1
#include <thread>
#include "mledstrip.h"
#include "mrgbcolor.h"
#include "mledstrip.h"
#include "meventhandler.h"
#include "mespnow.h"
#include <iostream>
#include "mbutton.h"
#include "mespcmd.h"
#include "mrocker.h"
#include "mnvs.h"
#include "mesptimer.h"
#include "uidriver.h"
#include "muicore.h"
#include "muitext.h"
#include "muiitem.h"
#include "keydriver.h"
#include "mrgbcolor.h"
#include "muiprogress.h"
#include "muiwindown.h"

using namespace std;

extern "C" void app_main(void)
{
    LedStrip* ledStrip = new LedStrip;
    ledStrip->init();
    ledStrip->setRGBAndUpdate(RGB_COLOR_WHITE);
    LcdDriver* lcd = new LcdDriver;
    MUicore::getInstance()->addLcd(lcd); 
    
    MUIWindown* window1 = new MUIWindown;
    window1->setBackGround(TFT_YELLOW);
    window1->show(true);
    MUiItem* item = new MUiItem(window1, 10,10,120,30);
    item->setText("item1kghhkjhkhkh",TFT_RED);
    item->setBackGround(TFT_WHITE);
    
        MUiItem item1(window1,10,44,120,30);
    item1.setBackGround(TFT_GREEN);
    item1.setText("item2",TFT_RED);
        item1.registerOnPressDown([&](MEventID id, MUIKeyID key, bool blongPress, uint32_t timerNum, bool brelease){
            if(brelease)
            {
                printf("set rgb yellow---\n");
                ledStrip->setRGBAndUpdate(RGB_COLOR_YELLOW);
            }
    });

        MUiItem item2(window1,10,78,120,30);
    item2.setBackGround(TFT_GREEN);
    item2.setText("item3",TFT_RED);
    item2.registerOnPressDown([&](MEventID id, MUIKeyID key, bool blongPress, uint32_t timerNum, bool brelease){
            if(brelease)
            {
                printf("set rgb OFF---\n");
                ledStrip->setRGBAndUpdate(RGB_COLOR_BLACK);
            }
    });

        MUiItem item3(window1,10,110,120,30);
    item3.setBackGround(TFT_GREEN);
    item3.setText("item4",TFT_RED);

    MUiText timerText(window1, 10,144,true,true);
    timerText.setText("TextTest", strlen("TextTest")+1, TFT_RED);
    MUIProgress progressBar(window1, 0,200,20,120);

    MUIProgress progressBar1(window1, 22,200,120,20);
    MUIWindown* window2 = new MUIWindown;
    window2->setBackGround(TFT_PINK);

    MUiItem* wifiItem[16];
    for(int i = 0, y = 0; i < 16; i++, y+=20)
    {
        wifiItem[i] = new MUiItem(MUiItem(window2, 0, y, 170, 20));
        wifiItem[i]->setBackGround(TFT_GREEN);
        wifiItem[i]->setText("default",TFT_RED);
        wifiItem[i]->registerOnPressDown([&](MEventID id, MUIKeyID key, bool blongPress, uint32_t timerNum, bool brelease){
        if(brelease)
        {
            ledStrip->setRGBAndUpdate(RGB_COLOR_BLACK);
            window1->show(true);
            window2->show(false);
        }
    });
    }

    item->registerOnPressDown([&](MEventID id, MUIKeyID key, bool blongPress, uint32_t timerNum, bool brelease){
        if(brelease)
        {
            ledStrip->setRGBAndUpdate(RGB_COLOR_WHITE);
            window1->show(false);
            window2->show(true);
        }
    });
    
    MButton buttonBoot(GPIO_NUM_0);
    MButton buttonUP(GPIO_NUM_15);
    MButton buttonDown(GPIO_NUM_16);
    MButton buttonMID(GPIO_NUM_17);
    MButton buttonLeft(GPIO_NUM_18);
    MButton buttonRight(GPIO_NUM_20);
    MButton buttonSet(GPIO_NUM_21);
    //MButton buttonReset(GPIO_NUM_48);
    
    //btinfo->blongPress, btinfo->bdoubleClick, btinfo->bbuttonRelease ,btinfo->timer
    int32_t count = 0;
    buttonSet.registerEventCb([&](uint32_t pin, bool blongPress, bool brelease, uint32_t holdtimer){
        count++;
        printf("button %lu prees, blongPress(%d),brelease(%d)holdtimer(%lu)\n", pin, blongPress,brelease,holdtimer);
        progressBar.setVal(count);
        progressBar1.setVal(count);
    });
    /*buttonReset.registerEventCb([&](uint32_t pin, bool blongPress, bool brelease, uint32_t holdtimer){
        count--;
        printf("button %lu prees, blongPress(%d),brelease(%d)holdtimer(%lu)\n", pin, blongPress,brelease,holdtimer);
        progressBar.setVal(count);
        progressBar1.setVal(count);
    });*/
    stKeyVal key;
    key.keyEnter = buttonMID.getPinNum();
    key.keyUp = buttonUP.getPinNum();
    key.keyDown = buttonDown.getPinNum();
    keyDriver::getInstance()->remappingKey(&key);
    
    MNvs* wifinvs = new MNvs();
    MWifiStation* station = MWifiStation::getInstance();
    station->registerWifiEventCb([&](esp_event_base_t eventBase, int32_t eventId, void* eventData){
        if(eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_START)
        {
            printf("eventId == WIFI_EVENT %ld\n", eventId);
            station->smartConfigStart();
            printf("smart config start %ld\n", eventId);
            //wifi->setSsidAndPasswd("TSD_SW9_SS5", "12345678");
            //wifi->connect();
            //lcd->fillRect( 0, 16, lcd->getWidth(), 16, TFT_BLACK);
            //lcd->drawString(0,16,"connect TSD_SW9_SS5",TFT_RED);
        }
        else if(eventBase == WIFI_EVENT && eventId == WIFI_EVENT_SCAN_DONE)
        {
            wifi_ap_record_t* apInfo = nullptr;
            uint16_t apNum = 0;

            station->wifiScanGetApRecords(&apInfo);
            station->wifiGetScanApNum(&apNum);
            for(int i = 0; i < apNum; i++)
            {
                printf("ssid: %s, rssi: %d\n",apInfo[i].ssid, apInfo[i].rssi);
                if(i < 16)
                {
                    wifiItem[i]->setText(reinterpret_cast<const char*>(apInfo[i].ssid),TFT_BLACK);
                }
            }
        }
        else if(eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_DISCONNECTED)
        {
            //lcd->fillRect( 0, 16, lcd->getWidth(), 16, TFT_BLACK);
            //lcd->drawString(0,16,"connect fail",TFT_RED);
            station->connect();
        }
        else if(eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_CONNECTED)
        {
            printf("connect successful\n");
            //lcd->fillRect( 0, 16, lcd->getWidth(), 16, TFT_BLACK);
            //lcd->drawString(0,16,"connect sucess",TFT_RED);
        }
        else if(eventBase == IP_EVENT && eventId == IP_EVENT_STA_GOT_IP)
        {
            uint32_t buff[8] = {0};
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) eventData;
            snprintf(reinterpret_cast<char*>(buff), sizeof(uint32_t)*8,IPSTR, IP2STR(&event->ip_info.ip));
            //(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
            //lcd->fillRect( 0, 32, lcd->getWidth(), 16, TFT_BLACK);
            //lcd->drawString(0,32,reinterpret_cast<char*> (buff),TFT_RED);
        }
        else if(eventBase == SC_EVENT && eventId == SC_EVENT_SCAN_DONE)
        {
            printf("1 smartconfig scan done\n");
        }
        else if(eventBase == SC_EVENT && eventId == SC_EVENT_FOUND_CHANNEL)
        {
            printf("2 smartconfig find channel\n");
        }
        else if(eventBase == SC_EVENT && eventId == SC_EVENT_GOT_SSID_PSWD)
        {
            printf("3 smartconfig getpasswd\n");
        wifi_config_t* wifi_config = (wifi_config_t*)eventData;

        uint8_t ssid[33] = { 0 };
        uint8_t password[65] = { 0 };
        memcpy(ssid, wifi_config->sta.ssid, sizeof(wifi_config->sta.ssid));
        memcpy(password, wifi_config->sta.password, sizeof(wifi_config->sta.password));
        printf( "SSID:%s\n", ssid);
        printf( "PASSWORD:%s\n", password);

        //ESP_ERROR_CHECK( esp_wifi_disconnect() );
        //ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, wifi_config) );
        //esp_wifi_connect();
        station->disconnect();
        station->setSsidAndPasswd(reinterpret_cast<const char*> (ssid), reinterpret_cast<const char*>(password));
        station->connect();
        }
        else if(eventBase == SC_EVENT && eventId == SC_EVENT_SEND_ACK_DONE)
        {
            printf("smart configstop\n");
            station->smartConfigStop();
        }

    });
    station->init();
    //station->wifiScanStart();

    time_t now;
    char strftime_buf[64];
    memset(strftime_buf,0,sizeof(strftime_buf));
    struct tm timeinfo;
    // 将时区设置为中国标准时间
    setenv("TZ", "CST-8", 1);
    tzset();
    while(true)
    {
        time(&now);
        localtime_r(&now, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        timerText.setText(strftime_buf, strlen(strftime_buf)+1, TFT_BLACK);
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}
#endif