#pragma once

#include <switch.h>
#include <string>
#include <vector>

#include "core/settings.hpp"

// Gửi ảnh lên backend /api/translate và đọc JSON trả về

namespace translate_client {

    /// Một đoạn chữ trên màn hình (toạ độ pixel của ảnh 1280x720)
    struct TextLine {
        int x = 0, y = 0, w = 0, h = 0;
        std::string source;
        std::string translation;
    };

    struct Translation {
        std::vector<TextLine> lines;
        std::string provider;   ///< provider phía server đã dịch (vd "gemini-primary")
    };

    /// Trả về chuỗi rỗng nếu thành công, ngược lại là thông báo lỗi ngắn để hiện lên GUI.
    std::string translate(const settings::Settings& settings, const u8* jpeg, size_t size, Translation& out);

}
