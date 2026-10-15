#pragma once

#include <string>

// Provider dịch mà người dùng chọn trong config.ini (provider=..., api_key=..., model=..., base_url=...).
// id trùng với header x-provider của backend. Thêm provider mới = thêm id vào SupportedProviders
// (và bảo đảm backend hỗ trợ id đó).

namespace providers {

    /// Provider có được overlay hỗ trợ không
    bool isSupported(const std::string& id);

    /// Danh sách id được hỗ trợ, dạng "gemini, ..." để đưa vào thông báo lỗi
    std::string supportedList();

    /// Provider này bắt buộc phải đặt model trong config.ini (backend không có model mặc định)
    bool requiresModel(const std::string& id);

    /// Provider này cần base_url trong config.ini (openai-compatible)
    bool requiresBaseUrl(const std::string& id);

    /// Lỗi nếu base_url không hợp lệ: phải là https://, không có ký tự lạ (rỗng nếu hợp lệ)
    std::string validateBaseUrl(const std::string& url);

    /// Lỗi nếu key không dùng được trong HTTP header (rỗng nếu hợp lệ)
    std::string validateKey(const std::string& key);

    /// Che API key: chỉ hiện 4 ký tự cuối, vd "****3JuG". Rỗng nếu chưa đặt.
    std::string maskKey(const std::string& key);

}
