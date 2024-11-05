#ifndef _XLCD_CONNECTION
#define _XLCD_CONNECTION

#include "mbedtls/base64.h"
#if defined(NO_SD)
#define XTOUCH_FS SPIFFS
#else
#define XTOUCH_FS SD
#endif
String xtouch_wifi_setup_decodeString(String inputString)
{
    const unsigned char *input = (const unsigned char *)inputString.c_str();
    size_t inputLength = strlen((const char *)input);

    // Calculate the exact maximum length for the output buffer
    size_t maxOutputLength = (inputLength * 3) / 4;
    unsigned char output[maxOutputLength + 1]; // +1 for the null terminator
    size_t outlen;

    int ret = mbedtls_base64_decode(output, maxOutputLength, &outlen, input, inputLength);

    if (ret == 0)
    {
        output[outlen] = '\0'; // Null-terminate the output
        return String((char *)output);
    }

    Serial.println("Failed to decode base64");
    return "";
}

bool xtouch_wifi_setup()
{
    DynamicJsonDocument wifiConfig = xtouch_filesystem_readJson(XTOUCH_FS, xtouch_paths_config);
    if (wifiConfig.isNull() || !wifiConfig.containsKey("ssid") || !wifiConfig.containsKey("pwd"))
    {
        lv_label_set_text(introScreenCaption, wifiConfig.isNull() ? LV_SYMBOL_SD_CARD " Missing config.json" : LV_SYMBOL_WARNING " Inaccurate config.json");
        lv_obj_set_style_text_color(introScreenCaption, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_timer_handler();
        lv_task_handler();
        return false;
    }

    String ssidB64String = xtouch_wifi_setup_decodeString(wifiConfig["ssid"].as<const char *>());
    String ssidPWDString = xtouch_wifi_setup_decodeString(wifiConfig["pwd"].as<const char *>());

    int timeout = wifiConfig.containsKey("timeout") ? wifiConfig["timeout"].as<int>() : 3000;

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssidB64String.c_str(), ssidPWDString.c_str());
    ConsoleInfo.println(F("[XTOUCH][CONNECTION] Connecting to WiFi .."));

    lv_label_set_text(introScreenCaption, LV_SYMBOL_WIFI " 连接中...");
    lv_obj_set_style_text_color(introScreenCaption, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_timer_handler();
    lv_task_handler();

    delay(timeout);
    wl_status_t status = WiFi.status();
    const char *statusText = "";
    lv_color_t statusColor = lv_color_hex(0x555555);

    bool reboot = false;
    while (status != WL_CONNECTED)
    {

        switch (status)
        {
        case WL_IDLE_STATUS:
            statusText = LV_SYMBOL_WIFI " 连接中...";
            statusColor = lv_color_hex(0x555555);
            break;

        case WL_NO_SSID_AVAIL:
            statusText = LV_SYMBOL_WARNING " WIFI名错误";
            statusColor = lv_color_hex(0xff0000);
            reboot = true;
            break;

            // case WL_CONNECTION_LOST:
            //     break;

        case WL_CONNECT_FAILED:
        case WL_DISCONNECTED:
            statusText = LV_SYMBOL_WARNING " WIFI密码错误";
            statusColor = lv_color_hex(0xff0000);
            reboot = true;
            break;

        default:
            break;
        }

        if (statusText != "")
        {

            lv_label_set_text(introScreenCaption, statusText);
            lv_obj_set_style_text_color(introScreenCaption, statusColor, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_timer_handler();
            lv_task_handler();
            delay(32);
        }

        if (reboot)
        {
            delay(3000);
            lv_label_set_text(introScreenCaption, LV_SYMBOL_REFRESH " 重启中");
            lv_timer_handler();
            lv_task_handler();
            ESP.restart();
        }
        status = WiFi.status();
    }

    WiFi.setTxPower(WIFI_POWER_19_5dBm); // https://github.com/G6EJD/ESP32-8266-Adjust-WiFi-RF-Power-Output/blob/main/README.md

    delay(1000);
    lv_label_set_text(introScreenCaption, LV_SYMBOL_WIFI " 已连接");
    lv_timer_handler();
    lv_task_handler();
    delay(1000);
    ConsoleInfo.print(F("[XTOUCH][CONNECTION] Connected to the WiFi network with IP: "));
    ConsoleInfo.println(WiFi.localIP());

    return true;
}

#endif