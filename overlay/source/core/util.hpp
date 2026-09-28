#pragma once

#include <switch.h>
#include <cstdio>
#include <string>

namespace util {

    /// Mã lỗi libnx dạng "0x1234" để hiển thị
    inline std::string resultToString(Result rc) {
        char buf[16];
        snprintf(buf, sizeof(buf), "0x%X", rc);
        return buf;
    }

}
