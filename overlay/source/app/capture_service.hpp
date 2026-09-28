#pragma once

#include <switch.h>
#include <string>

// Chạy việc chụp màn hình (và sau này là dịch) trên một thread nền,
// để không chặn thread input hay thread vẽ của Tesla.

namespace capture_service {

    void start();
    void stop();

    /// Yêu cầu chụp. Không chặn; trả về false nếu đang bận xử lý lần trước.
    bool request();

    /// Trạng thái gần nhất để hiển thị lên GUI
    std::string status();

    /// Tăng mỗi khi status thay đổi, để GUI biết lúc nào cần cập nhật
    u32 statusVersion();

}
