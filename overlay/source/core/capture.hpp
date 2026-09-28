#pragma once

#include <switch.h>

// Chụp màn hình qua service caps:sc

namespace capture {

    enum class Source {
        Game,    ///< Chỉ lớp game (không dính overlay)
        Screen,  ///< Toàn màn hình (fallback khi không có game, có dính overlay)
    };

    struct Frame {
        const u8* data = nullptr;
        size_t    size = 0;
        Source    source = Source::Game;
    };

    Result init();
    void   exit();

    /**
     * Chụp màn hình ra JPEG 1280x720.
     * Dữ liệu nằm trong buffer nội bộ, hợp lệ tới lần gọi tiếp theo.
     */
    Result captureJpeg(Frame& out);

    const char* sourceName(Source source);

}
