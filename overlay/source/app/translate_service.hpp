#pragma once

#include <switch.h>
#include <string>

#include "core/translate_client.hpp"

// Pipeline chạy trên thread nền: chụp màn hình → gửi server → lưu bản dịch.
// Không chặn thread input hay thread vẽ của Tesla.

namespace translate_service {

    void start();
    void stop();

    /// Yêu cầu chụp và dịch. Không chặn; trả về false nếu đang bận xử lý lần trước.
    bool request();

    /// Yêu cầu kiểm tra mạng (kết quả hiện ở status). Không chặn.
    bool requestDiagnose();

    /// Trạng thái gần nhất để hiển thị lên GUI
    std::string status();
    /// Tăng mỗi khi status thay đổi, để GUI biết lúc nào cần cập nhật
    u32 statusVersion();

    /// Bản dịch gần nhất (bản sao)
    translate_client::Translation translation();
    /// Tăng mỗi khi có bản dịch mới
    u32 translationVersion();

}
