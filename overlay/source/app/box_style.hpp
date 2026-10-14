#pragma once

#include <switch.h>

// Kiểu hiển thị của box bản dịch: độ đậm (độ đặc) của nền box, đổi được trong overlay và lưu vào
// config.ini (box_opacity=0..100). 0 = trong suốt hoàn toàn, 100 = đặc.

namespace box_style {

    /// Đọc box_opacity từ config.ini (mặc định config::DefaultBoxOpacity nếu thiếu hoặc không hợp lệ)
    void load();

    int opacityPercent();

    /// Đặt độ đậm (0-100). Chỉ đổi trong bộ nhớ để xem trước; gọi save() để ghi file.
    void setOpacityPercent(int percent);

    /// Ghi box_opacity vào config.ini. Trả về false nếu không ghi được.
    bool save();

    /// Alpha 0-15 dùng cho tsl::Color (RGBA4444)
    u8 alpha();

    /// Tăng mỗi khi giá trị thay đổi, để GUI cập nhật nhãn
    u32 version();

}
