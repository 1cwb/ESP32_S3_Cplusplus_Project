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
#include "muiItem.h"
#include "keydriver.h"

using namespace std;

extern "C" void app_main(void)
{
    LcdDriver lcd;
    MUicore::getInstance()->addLcd(&lcd); 
    MUicore::getInstance()->setBackGround(TFT_YELLOW);
    MUicore::getInstance()->updateUiNotify(nullptr);
    MUiItem item(10,10,120,30);
    item.setBackGround(TFT_GREEN);
    item.setText("item1",TFT_RED);
    //item.setFocused(true);
    item.setCanbefocus(false);
    item.registerOnPressDown([&](MEventID id, MUIKeyID key, bool blongPress, bool bdoubleClick, uint32_t timerNum, bool brelease){

    });
    
        MUiItem item1(10,44,120,30);
    item1.setBackGround(TFT_GREEN);
    item1.setText("item2",TFT_RED);
    item1.setCanbefocus(false);

        MUiItem item2(10,78,120,30);
    item2.setBackGround(TFT_GREEN);
    item2.setText("item3",TFT_RED);

        MUiItem item3(10,110,120,30);
    item3.setBackGround(TFT_GREEN);
    item3.setText("item4",TFT_RED);

    MUiText timerText(10,144,true,false);
    timerText.setText("TextTest", strlen("TextTest")+1, TFT_BLACK);


    MButton button1(GPIO_NUM_0);
    MButton buttonUP(GPIO_NUM_4);
    MButton buttonDown(GPIO_NUM_3);
    MButton buttonMID(GPIO_NUM_5);
    MButton buttonx(GPIO_NUM_1);
    MButton buttonu(GPIO_NUM_2);
    MButton button6(GPIO_NUM_6);
    MButton button7(GPIO_NUM_7);
    MButton button8(GPIO_NUM_8);
    stKeyVal key;
    key.keyEnter = button1.getPinNum();
    key.keyUp = buttonx.getPinNum();
    key.keyDown = buttonu.getPinNum();
    keyDriver::getInstance()->remappingKey(&key);
    int32_t i = -100;
    while(true)
    {
        vTaskDelay(20/portTICK_PERIOD_MS);
        /*for(int x = 0; x < 170; x+=10)
        {
            for(int j = 0; j < 320 - 16; j+=10)
            {
                item.setIntNum(i, TFT_PINK+i);
                item.setXY(x,j);
                i++;
                vTaskDelay(100/portTICK_PERIOD_MS);
            }
        }*/
    }
}
#endif

#if 0
#include "mbutton.h"
using namespace std;

extern "C" void app_main(void)
{
    MButton button1(GPIO_NUM_0);
    MButton buttonUP(GPIO_NUM_4);
    MButton buttonDown(GPIO_NUM_3);
    MButton buttonMID(GPIO_NUM_5);
    MButton buttonx(GPIO_NUM_1);
    MButton buttonu(GPIO_NUM_2);
    MButton button6(GPIO_NUM_6);
    MButton button7(GPIO_NUM_7);
    MButton button8(GPIO_NUM_8);
    while(true)
    {
        vTaskDelay(20/portTICK_PERIOD_MS);
    }
}

#endif