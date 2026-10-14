#pragma once

#include <switch.h>
#include <string>
#include <vector>

// Lựa chọn provider và model của người dùng, đọc/ghi từ config.ini (provider=..., model=...).
// API key không chọn được trong overlay (không có bàn phím), vẫn đặt trong config.ini.

namespace provider_choice {

    struct Provider {
        const char* id;     ///< giá trị x-provider và provider=... trong config.ini
        const char* label;  ///< tên hiển thị
    };

    struct Model {
        const char* id;
        const char* note;   ///< ghi chú ngắn hiện bên phải
    };

    const std::vector<Provider>& providers();

    /// Model gợi ý sẵn cho provider. Rỗng nếu không có (openrouter, openai-compatible: đặt trong config.ini)
    const std::vector<Model>& presets(const std::string& providerId);

    /// Model mặc định của backend cho provider ("" nếu backend bắt buộc phải đặt model)
    std::string defaultModel(const std::string& providerId);

    /// Tên hiển thị của provider ("Gemini"...); trả lại chính id nếu lạ
    std::string label(const std::string& providerId);

    /// Provider đang chọn (config.ini, mặc định gemini; alias anthropic/gpt được đổi về claude/openai)
    std::string currentProvider();

    /// Model đang chọn trong config.ini ("" = dùng mặc định của provider)
    std::string currentModel();

    /// Chọn provider. Nếu khác provider cũ thì model được đặt lại về mặc định. Trả về false nếu không ghi được file.
    bool setProvider(const std::string& id);

    /// Chọn model ("" = mặc định). Trả về false nếu không ghi được file.
    bool setModel(const std::string& id);

    /// Tăng mỗi khi lựa chọn thay đổi, để GUI cập nhật nhãn
    u32 version();

}
