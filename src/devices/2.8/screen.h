#ifndef _XLCD_SCREEN
#define _XLCD_SCREEN

#define LGFX_USE_V1      // set to use new version of library
#include <LovyanGFX.hpp> // main library

#define SD_SCK 18
#define SD_MISO 19
#define SD_MOSI 23
#define SD_CS 5

#define LCD_BACK_LIGHT_PIN 0

// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0 0

// use 12 bit precission for LEDC timer
#define LEDC_TIMER_12_BIT 12

// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ 5000

static const uint16_t screenWidth = 240;
static const uint16_t screenHeight = 320;
#define TFT_WIDTH 240
#define TFT_HEIGHT 320



// static lv_disp_draw_buf_t draw_buf;
// static lv_color_t *buf1;
// static lv_color_t *buf2;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[2][ screenWidth * 10 ];
// static lv_color_t buf[4096];


class LGFX : public lgfx::LGFX_Device
{

    lgfx::Panel_ST7789 _panel_instance; // ST7789UI
    lgfx::Bus_Parallel8 _bus_instance;  // MCU8080 8B

public:
    LGFX(void)
    {
        {
            auto cfg = _bus_instance.config();
            cfg.freq_write = 25000000;
            cfg.pin_wr = 4;
            cfg.pin_rd = 2;
            cfg.pin_rs = 16;

            cfg.pin_d0 = 15;
            cfg.pin_d1 = 13;
            cfg.pin_d2 = 12;
            cfg.pin_d3 = 14;
            cfg.pin_d4 = 27;
            cfg.pin_d5 = 25;
            cfg.pin_d6 = 33;
            cfg.pin_d7 = 32;

            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        {
            auto cfg = _panel_instance.config();

            cfg.pin_cs = 17;
            cfg.pin_rst = -1;
            cfg.pin_busy = -1;

            cfg.panel_width = 240;
            cfg.panel_height = 320;
            cfg.offset_x = 0;
            cfg.offset_y = 0;
            cfg.offset_rotation = 0;
            // cfg.dummy_read_pixel = 8;
            // cfg.dummy_read_bits = 1;
            cfg.readable = false;
            cfg.invert = false;
            cfg.rgb_order = false;
            cfg.dlen_16bit = false;
            cfg.bus_shared = true;

            _panel_instance.config(cfg);
        }

        setPanel(&_panel_instance);
    }
};

static LGFX tft; // declare display variable



#include "ui/ui.h"
#include "touch.h"
#include "xtouch/globals.h"

bool xtouch_screen_touchFromPowerOff = false;

void xtouch_screen_ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255)
{
    // calculate duty, 4095 from 2 ^ 12 - 1
    uint32_t duty = (4095 / valueMax) * min(value, valueMax);

    // write duty to LEDC
    ledcWrite(channel, duty);
}

void xtouch_screen_setBrightness(byte brightness)
{
    xtouch_screen_ledcAnalogWrite(LEDC_CHANNEL_0, brightness);
}

void xtouch_screen_setBackLedOff()
{
    pinMode(4, OUTPUT);
    pinMode(16, OUTPUT);
    pinMode(17, OUTPUT);
    digitalWrite(4, HIGH);
    digitalWrite(16, HIGH);
    digitalWrite(17, HIGH); // The LEDs are "active low", meaning HIGH == off, LOW == on
}

void xtouch_screen_wakeUp()
{
    lv_timer_reset(xtouch_screen_onScreenOffTimer);
    xtouch_screen_touchFromPowerOff = false;
    loadScreen(0);
    xtouch_screen_setBrightness(xTouchConfig.xTouchBacklightLevel);
}
 
void xtouch_screen_onScreenOff(lv_timer_t *timer)
{
    // if (bambuStatus.print_status == XTOUCH_PRINT_STATUS_RUNNING)
    // {
    //     return;
    // }

    if (xTouchConfig.xTouchTFTOFFValue < XTOUCH_LCD_MIN_SLEEP_TIME)
    {
        return;
    }

    ConsoleInfo.println("[XTouch][SCREEN] Screen Off");
    xtouch_screen_setBrightness(0);
    xtouch_screen_touchFromPowerOff = true;
}

void xtouch_screen_setupScreenTimer()
{
    xtouch_screen_onScreenOffTimer = lv_timer_create(xtouch_screen_onScreenOff, xTouchConfig.xTouchTFTOFFValue * 1000 * 60, NULL);
    lv_timer_pause(xtouch_screen_onScreenOffTimer);
}

void xtouch_screen_startScreenTimer()
{
    lv_timer_resume(xtouch_screen_onScreenOffTimer);
}

void xtouch_screen_setScreenTimer(uint32_t period)
{
    lv_timer_set_period(xtouch_screen_onScreenOffTimer, period);
}

void xtouch_screen_invertColors()
{
    tft.invertDisplay(xTouchConfig.xTouchTFTInvert);
}

byte xtouch_screen_getTFTFlip()
{
    byte val = xtouch_eeprom_read(XTOUCH_EEPROM_POS_TFTFLIP);
    xTouchConfig.xTouchTFTFlip = val;
    return val;
}

void xtouch_screen_setTFTFlip(byte mode)
{
    xTouchConfig.xTouchTFTFlip = mode;
    xtouch_eeprom_write(XTOUCH_EEPROM_POS_TFTFLIP, mode);
}

void xtouch_screen_toggleTFTFlip()
{
    xtouch_screen_setTFTFlip(!xtouch_screen_getTFTFlip());
    xtouch_resetTouchConfig();
}

void xtouch_screen_setupTFTFlip()
{
    byte eepromTFTFlip = xtouch_screen_getTFTFlip();
    tft.setRotation(eepromTFTFlip == 3 ? 1 : 3);
    x_touch_touchScreen.setRotation(eepromTFTFlip == 3 ? 1 : 3);
}

void xtouch_screen_dispFlush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    if (tft.getStartCount() == 0)
    {   // Processing if not yet started
        tft.startWrite();
    }
    tft.pushImageDMA( area->x1
                    , area->y1
                    , area->x2 - area->x1 + 1
                    , area->y2 - area->y1 + 1
                    , ( lgfx::swap565_t* )&color_p->full);
    lv_disp_flush_ready( disp );
}

void xtouch_screen_touchRead(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    if (x_touch_touchScreen.tirqTouched() && x_touch_touchScreen.touched())
    {
        lv_timer_reset(xtouch_screen_onScreenOffTimer);
        // dont pass first touch after power on
        if (xtouch_screen_touchFromPowerOff)
        {
            xtouch_screen_wakeUp();
            while (x_touch_touchScreen.touched())
                ;
            return;
        }

        #if not defined(SKIP_TP_CAL)
        ScreenPoint sp = ScreenPoint();
        TS_Point p = x_touch_touchScreen.getPoint();
        sp = getScreenCoords(p.x, p.y);
        data->state = LV_INDEV_STATE_PR;
        data->point.x = sp.x;
        data->point.y = sp.y;
        #else
        TS_Point p = x_touch_touchScreen.getPoint();
        data->state = LV_INDEV_STATE_PR;
        data->point.x = p.x;
        data->point.y = p.y;
        #endif
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }
}

void xtouch_screen_setup()
{

    tft.begin();
    ConsoleInfo.println("[XTouch][SCREEN] Setup");
    lv_init();

    // pinMode(XPT2046_CS, OUTPUT);
    // pinMode(TFT_CS, OUTPUT);
    pinMode(SD_CS, OUTPUT);

    // digitalWrite(XPT2046_CS, HIGH); // Touch controller chip select (if used)
    // digitalWrite(TFT_CS, HIGH);     // TFT screen chip select
    digitalWrite(SD_CS, HIGH);      // SD card chips select, must use GPIO 5 (ESP32 SS)

    // xtouch_screen_setBackLedOff();
    pinMode(0, OUTPUT);
    digitalWrite(0, HIGH);
    
    tft.fillScreen(TFT_BLACK);
    x_touch_touchScreen.begin();
    ConsoleInfo.println("TouchScreen Setup Done");

    ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
    ledcAttachPin(LCD_BACK_LIGHT_PIN, LEDC_CHANNEL_0);
    
    xtouch_screen_setupTFTFlip();
    xtouch_screen_setBrightness(255);


    // // xtouch_screen_setBrightness(255);
    // lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(TFT_HEIGHT * 150 * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    // lv_color_t *buf2 = (lv_color_t *)heap_caps_malloc(TFT_HEIGHT * 150 * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    lv_disp_draw_buf_init( &draw_buf, buf[0], buf[1], screenWidth * 10 );
    // lv_disp_draw_buf_init(&draw_buf, buf1, buf2, TFT_HEIGHT * 150);
    // lv_disp_draw_buf_init(&draw_buf, buf1, NULL, TFT_HEIGHT * 150);


    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenHeight;
    disp_drv.ver_res = screenWidth;
    disp_drv.flush_cb = xtouch_screen_dispFlush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);


    /*Initialize the (dummy) input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = xtouch_screen_touchRead;
    lv_indev_drv_register(&indev_drv);

    /*Initialize the graphics library */
    LV_EVENT_GET_COMP_CHILD = lv_event_register_id();

    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);

    initTopLayer();
}

#endif