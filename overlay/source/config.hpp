#pragma once

#include <switch.h>

// Cấu hình chung của overlay: đường dẫn trên thẻ SD và các hằng số

namespace config {

    constexpr const char* AppName = "Translate";

#ifdef APP_VERSION_STR
    constexpr const char* AppVersion = "v" APP_VERSION_STR;
#else
    constexpr const char* AppVersion = "dev";
#endif

    // Thư mục chứa dữ liệu của overlay (font, key, ảnh chụp...)
    constexpr const char* DataDir     = "sdmc:/config/translate";
    constexpr const char* CapturePath = "sdmc:/config/translate/capture.jpg";
    // URL server, token, ngôn ngữ đích (xem config.ini.example)
    constexpr const char* SettingsPath = "sdmc:/config/translate/config.ini";

    // Thời gian tối đa chờ server dịch (giây)
    constexpr long TranslateTimeoutSec = 30;

    // Tổ hợp phím chụp khi overlay đang ẩn (trong lúc chơi game)
    constexpr u64 CaptureHotkey = HidNpadButton_L | HidNpadButton_R | HidNpadButton_A;
    constexpr const char* CaptureHotkeyName = "L+R+ZR";

}
