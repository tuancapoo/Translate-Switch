#pragma once

#include <switch.h>
#include <string>
#include <vector>

// HTTP(S) client tối giản trên libcurl

namespace http {

    struct Response {
        long status = 0;       ///< HTTP status, 0 nếu không kết nối được
        std::string body;
        std::string error;     ///< Lỗi mạng/TLS của curl (rỗng nếu request tới được server)
    };

    /// Khởi tạo socket + curl. Gọi trong initServices (cần sm session).
    Result init();
    void   exit();

    /// Kiểm tra mạng để debug: trạng thái kết nối, IP, và thử phân giải tên miền của url
    std::string diagnose(const std::string& url);

    /// POST dữ liệu nhị phân. headers dạng "Name: value".
    Response post(const std::string& url, const std::vector<std::string>& headers,
                  const void* data, size_t size, long timeoutSec);

}
