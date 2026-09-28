#include "core/capture.hpp"

namespace capture {

    namespace {

        // JPEG 1280x720 thường 100-400KB
        alignas(0x1000) u8 s_jpegBuffer[0x80000];

        constexpr s64 Timeout = 10'000'000'000; // 10 giây (ns)

        Result captureFrom(ViLayerStack stack, u64& outSize) {
            return capsscCaptureJpegScreenShot(&outSize, s_jpegBuffer, sizeof(s_jpegBuffer), stack, Timeout);
        }

    }

    Result init() {
        return capsscInitialize();
    }

    void exit() {
        capsscExit();
    }

    Result captureJpeg(Frame& out) {
        u64 size = 0;

        // Ưu tiên lớp game; khi đang ở HOME (không có game) thì chụp toàn màn hình
        Source source = Source::Game;
        Result rc = captureFrom(ViLayerStack_ApplicationForDebug, size);
        if (R_FAILED(rc)) {
            source = Source::Screen;
            rc = captureFrom(ViLayerStack_Screenshot, size);
        }

        if (R_SUCCEEDED(rc))
            out = { s_jpegBuffer, size, source };
        return rc;
    }

    const char* sourceName(Source source) {
        return source == Source::Game ? "game" : "screen";
    }

}
