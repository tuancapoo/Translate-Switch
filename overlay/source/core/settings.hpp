#pragma once

#include <string>

// Cấu hình người dùng đọc từ file INI trên thẻ SD (config::SettingsPath)
//
//   url=https://<site>.netlify.app/api/translate
//   token=<APP_TOKEN của server>
//   lang=Vietnamese
//   provider=gemini          (tuỳ chọn) provider dịch, hiện hỗ trợ: gemini
//   api_key=<key của bạn>    (tuỳ chọn) có key thì gửi kèm request; bỏ trống thì server dùng key của nó
//   model=<tên model>        (tuỳ chọn) bỏ trống để dùng model mặc định của provider

namespace settings {

    struct Settings {
        std::string url;
        std::string token;
        std::string lang = "Vietnamese";
        std::string provider = "gemini";
        std::string apiKey;
        std::string model;
    };

    /// Đọc file cấu hình. Trả về chuỗi rỗng nếu thành công, ngược lại là thông báo lỗi.
    std::string load(const char* path, Settings& out);

    /// Đọc một giá trị bất kỳ (rỗng nếu không có)
    std::string readValue(const char* path, const std::string& key);

    /// Ghi một giá trị: thay dòng "key=..." nếu có, không thì thêm vào cuối. Giữ nguyên các dòng khác.
    bool writeValue(const char* path, const std::string& key, const std::string& value);

}
