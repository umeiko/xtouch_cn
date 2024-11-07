#ifndef _XLCD_TOUCH
#define _XLCD_TOUCH

// #include <XPT2046_Touchscreen.h>

// #define XPT2046_IRQ 36
// #define XPT2046_MOSI 32
// #define XPT2046_MISO 39
// #define XPT2046_CLK 25
// #define XPT2046_CS 33
/*定义触摸屏引脚*/
#include "CST820.h"
#define I2C_SDA 33
#define I2C_SCL 32
#define TP_RST 25
#define TP_INT 21
// SPIClass x_touch_spi = SPIClass(HSPI);
// XPT2046_Touchscreen x_touch_touchScreen(XPT2046_CS, XPT2046_IRQ);
CST820 x_touch_touchScreen(I2C_SDA, I2C_SCL, TP_RST, TP_INT); /* 触摸实例 */
XTouchPanelConfig x_touch_touchConfig;

#if defined(NO_SD)
#define XTOUCH_FS SPIFFS
#else
#define XTOUCH_FS SD
#endif

class ScreenPoint
{
public:
    int16_t x;
    int16_t y;

    // default constructor
    ScreenPoint()
    {
    }

    ScreenPoint(int16_t xIn, int16_t yIn)
    {
        x = xIn;
        y = yIn;
    }
};

ScreenPoint getScreenCoords(int16_t x, int16_t y)
{
    int16_t xCoord = round((x * x_touch_touchConfig.xCalM) + x_touch_touchConfig.xCalC);
    int16_t yCoord = round((y * x_touch_touchConfig.yCalM) + x_touch_touchConfig.yCalC);
    if (xCoord < 0)
        xCoord = 0;
    if (xCoord >= 320)
        xCoord = 320 - 1;
    if (yCoord < 0)
        yCoord = 0;
    if (yCoord >= 240)
        yCoord = 240 - 1;
    return (ScreenPoint(xCoord, yCoord));
}

void xtouch_loadTouchConfig(XTouchPanelConfig &config)
{
    // Open file for reading
    File file = xtouch_filesystem_open(XTOUCH_FS, xtouch_paths_touch);

    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/v6/assistant to compute the capacity.
    StaticJsonDocument<512> doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, file);
    if (error)
        ConsoleError.println(F("[XTouch][Touch] Failed to read touch config"));

    config.xCalM = doc["xCalM"].as<float>();
    config.yCalM = doc["yCalM"].as<float>();
    config.xCalC = doc["xCalC"].as<float>();
    config.yCalC = doc["yCalC"].as<float>();

    file.close();
}

void xtouch_saveTouchConfig(XTouchPanelConfig &config)
{
    StaticJsonDocument<512> doc;
    doc["xCalM"] = config.xCalM;
    doc["yCalM"] = config.yCalM;
    doc["xCalC"] = config.xCalC;
    doc["yCalC"] = config.yCalC;
    xtouch_filesystem_writeJson(XTOUCH_FS, xtouch_paths_touch, doc);
}

void xtouch_resetTouchConfig()
{
    ConsoleInfo.println(F("[XTouch][FS] Resetting touch config"));
    xtouch_filesystem_deleteFile(XTOUCH_FS, xtouch_paths_touch);
    delay(500);
    ESP.restart();
}

bool hasTouchConfig()
{
    ConsoleInfo.println(F("[XTouch][FS] Checking for touch config"));
    return xtouch_filesystem_exist(XTOUCH_FS, xtouch_paths_touch);
}

void xtouch_touch_setup()
{
    if (hasTouchConfig())
    {
        ConsoleInfo.println(F("[XTouch][TOUCH] Load from disk"));
        xtouch_loadTouchConfig(x_touch_touchConfig);
    }
    else
    {
        ConsoleInfo.println(F("[XTouch][TOUCH] Touch Setup"));
        TS_Point p;
        int16_t x1, y1, x2, y2;

        lv_label_set_text(introScreenCaption, "用触控笔点击  " LV_SYMBOL_PLUS "  校准触摸屏");
        lv_timer_handler();

        // wait for no touch
        while (x_touch_touchScreen.touched())
            ;
        tft.drawFastHLine(0, 10, 20, TFT_WHITE);
        tft.drawFastVLine(10, 0, 20, TFT_WHITE);
        while (!x_touch_touchScreen.touched())
            ;
        delay(50);
        p = x_touch_touchScreen.getPoint();
        x1 = p.x;
        y1 = p.y;
        tft.drawFastHLine(0, 10, 20, TFT_BLACK);
        tft.drawFastVLine(10, 0, 20, TFT_BLACK);
        delay(500);

        while (x_touch_touchScreen.touched())
            ;
        tft.drawFastHLine(300, 230, 20, TFT_WHITE);
        tft.drawFastVLine(310, 220, 20, TFT_WHITE);

        while (!x_touch_touchScreen.touched())
            ;
        delay(50);
        p = x_touch_touchScreen.getPoint();
        x2 = p.x;
        y2 = p.y;
        tft.drawFastHLine(300, 230, 20, TFT_BLACK);
        tft.drawFastVLine(310, 220, 20, TFT_BLACK);

        int16_t xDist = 320 - 40;
        int16_t yDist = 240 - 40;

        x_touch_touchConfig.xCalM = (float)xDist / (float)(x2 - x1);
        x_touch_touchConfig.xCalC = 20.0 - ((float)x1 * x_touch_touchConfig.xCalM);
        // y
        x_touch_touchConfig.yCalM = (float)yDist / (float)(y2 - y1);
        x_touch_touchConfig.yCalC = 20.0 - ((float)y1 * x_touch_touchConfig.yCalM);

        xtouch_saveTouchConfig(x_touch_touchConfig);

        loadScreen(-1);
    }
}

#endif