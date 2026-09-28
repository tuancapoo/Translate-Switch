#pragma once

#include <cstddef>

// Đọc/ghi file trên thẻ SD

namespace storage {

    /// Tạo thư mục (và các thư mục cha) nếu chưa có
    void ensureDir(const char* path);

    /// Ghi toàn bộ dữ liệu ra file, tự tạo thư mục cha. Trả về true nếu thành công.
    bool writeFile(const char* path, const void* data, size_t size);

}
