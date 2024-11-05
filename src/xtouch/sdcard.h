#ifndef _XLCD_SDCARD
#define _XLCD_SDCARD

#include "FS.h"
#if not defined(NO_SD)
#include "SD.h"
#else
#include "SPIFFS.h"
#endif
#include <ArduinoJson.h>
#include <Arduino.h>

#if not defined(NO_SD)
bool xtouch_sdcard_setup()
{
    if (!SD.begin())
    {
        lv_label_set_text(introScreenCaption, LV_SYMBOL_SD_CARD " 请插入配置好账户信息的TF卡");
        lv_obj_set_style_text_color(introScreenCaption, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_timer_handler();

        ConsoleError.println("[XTouch][SD] Card Mount Failed");
        return false;
    }

    lv_obj_set_style_text_color(introScreenCaption, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);

    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE)
    {
        ConsoleError.println("[XTouch][SD] No SD card attached");
        return false;
    }

    ConsoleInfo.print("XTouch][SD] SD Card Type: ");

    if (cardType == CARD_MMC)
    {
        ConsoleInfo.println("[XTouch][SD] MMC");
    }
    else if (cardType == CARD_SD)
    {
        ConsoleInfo.println("[XTouch][SD] SDSC");
    }
    else if (cardType == CARD_SDHC)
    {
        ConsoleInfo.println("[XTouch][SD] SDHC");
    }
    else
    {
        ConsoleInfo.println("[XTouch][SD] UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    ConsoleInfo.printf("[XTouch][SD] SD Card Size: %lluMB\n", cardSize);
    xtouch_filesystem_mkdir(SD, xtouch_paths_root);

    return true;
}
#else
void plot_fs_msg(){
      // 获取SPIFFS的信息
  uint32_t totalBytes = SPIFFS.totalBytes();
  uint32_t usedBytes = SPIFFS.usedBytes();
  // 计算剩余空间
  uint32_t freeBytes = totalBytes - usedBytes;

  // 打印信息
  Serial.print("Total bytes: ");
  Serial.println(totalBytes);
  Serial.print("Used bytes: ");
  Serial.println(usedBytes);
  Serial.print("Free bytes: ");
  Serial.println(freeBytes);
}

bool xtouch_sdcard_setup(){
    lv_label_set_text(introScreenCaption, LV_SYMBOL_SD_CARD "尝试初始化内部存储器");
    lv_obj_set_style_text_color(introScreenCaption, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_timer_handler();
    // File root = SPIFFS.open("/");
    // File file = root.openNextFile();
    // while (file) {
    //     if (file.isDirectory()) {
    //     Serial.print("DIR : ");
    //     Serial.println(file.name());
    //     } else {
    //     Serial.print("FILE: ");
    //     Serial.println(file.name());
    //     }
    //     file = root.openNextFile();
    // }
    // root.close();
    // file.close();
    plot_fs_msg();

    // xtouch_filesystem_mkdir(SPIFFS, xtouch_paths_root);
    if (!xtouch_filesystem_exist(SPIFFS, xtouch_paths_config)){
        lv_label_set_text(introScreenCaption, LV_SYMBOL_USB "请连接电脑软件进行初始化设置");
        lv_obj_set_style_text_color(introScreenCaption, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_timer_handler();
        return false;
    }
    return true;
}
#endif
#endif