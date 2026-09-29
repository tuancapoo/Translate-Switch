#pragma once

#include <string>

// Cấu hình người dùng đọc từ file INI trên thẻ SD (config::SettingsPath)
//
//   url=https://<site>.netlify.app/api/translate
//   token=<APP_TOKEN của server>
//   lang=Vietnamese

namespace settings {

    struct Settings {
        std::string url;
        std::string token;
        std::string lang = "Vietnamese";
    };

    /// Đọc file cấu hình. Trả về chuỗi rỗng nếu thành công, ngược lại là thông báo lỗi.
    std::string load(const char* path, Settings& out);

}
