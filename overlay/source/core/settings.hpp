#pragma once

#include <string>

// Cấu hình người dùng đọc từ file INI trên thẻ SD (config::SettingsPath)
//
//   url=https://<site>.netlify.app/api/translate
//   token=<APP_TOKEN của server>
//   lang=Vietnamese
//   provider=gemini          provider dịch: gemini, claude, openai, openrouter, openai-compatible
//   api_key=<key của bạn>    BẮT BUỘC: key của provider đã chọn
//   model=<tên model>        tuỳ chọn; bắt buộc với openrouter và openai-compatible
//   base_url=https://...     chỉ cho openai-compatible

namespace settings {

    struct Settings {
        std::string url;
        std::string token;
        std::string lang = "Vietnamese";
        std::string provider = "gemini";
        std::string apiKey;
        std::string model;
        std::string baseUrl;   ///< chỉ dùng với provider=openai-compatible
    };

    /// Đọc file cấu hình. Trả về chuỗi rỗng nếu thành công, ngược lại là thông báo lỗi.
    std::string load(const char* path, Settings& out);

    /// Đọc một giá trị bất kỳ (rỗng nếu không có)
    std::string readValue(const char* path, const std::string& key);

    /// Ghi một giá trị: thay dòng "key=..." nếu có, không thì thêm vào cuối. Giữ nguyên các dòng khác.
    bool writeValue(const char* path, const std::string& key, const std::string& value);

}
