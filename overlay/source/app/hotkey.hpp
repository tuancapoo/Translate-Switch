#pragma once

#include <switch.h>
#include <string>

// Tổ hợp phím chụp & dịch. Đọc/ghi từ config.ini (hotkey=L+R+ZR), dùng được từ mọi thread.

namespace hotkey {

    /// Đọc hotkey từ config.ini (mặc định config::DefaultHotkey nếu thiếu hoặc không hợp lệ)
    void load();

    u64 get();

    /// Kiểm tra tổ hợp có dùng được không. Trả về lỗi (rỗng nếu hợp lệ).
    std::string validate(u64 keys);

    /// Đặt và lưu vào config.ini. Trả về false nếu ghi file thất bại (hotkey vẫn được dùng tạm).
    bool save(u64 keys);

    /// "L+R+ZR" (định dạng trong config.ini)
    std::string toString(u64 keys);

    /// Icon nút Nintendo, vd " +  + " (vẽ bằng font mở rộng của hệ thống)
    std::string toGlyphs(u64 keys);

    /// Các nút được phép dùng trong hotkey
    u64 allowedMask();

}
