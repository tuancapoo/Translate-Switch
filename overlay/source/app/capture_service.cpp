#include "app/capture_service.hpp"

#include "config.hpp"
#include "core/capture.hpp"
#include "core/storage.hpp"
#include "core/util.hpp"

#include <atomic>
#include <mutex>

namespace capture_service {

    namespace {

        Thread s_thread;
        UEvent s_requestEvent;
        std::atomic<bool> s_running = false;
        std::atomic<bool> s_busy = false;

        std::mutex s_statusMutex;
        std::string s_status = "Sẵn sàng";
        std::atomic<u32> s_statusVersion = 0;

        void setStatus(std::string text) {
            {
                std::scoped_lock lock(s_statusMutex);
                s_status = std::move(text);
            }
            s_statusVersion++;
        }

        void runCapture() {
            setStatus("Đang chụp...");

            capture::Frame frame;
            Result rc = capture::captureJpeg(frame);
            if (R_FAILED(rc)) {
                setStatus("Lỗi chụp: " + util::resultToString(rc));
                return;
            }

            if (!storage::writeFile(config::CapturePath, frame.data, frame.size)) {
                setStatus("Lỗi ghi file SD");
                return;
            }

            setStatus("✓ " + std::to_string(frame.size / 1024) + " KB (" + capture::sourceName(frame.source) + ")");
        }

        void threadMain(void*) {
            while (true) {
                waitSingle(waiterForUEvent(&s_requestEvent), UINT64_MAX);
                if (!s_running)
                    break;

                runCapture();
                s_busy = false;
            }
        }

    }

    void start() {
        if (s_running)
            return;

        ueventCreate(&s_requestEvent, true);
        s_running = true;
        threadCreate(&s_thread, threadMain, nullptr, nullptr, 0x8000, 0x2C, -2);
        threadStart(&s_thread);
    }

    void stop() {
        if (!s_running)
            return;

        s_running = false;
        ueventSignal(&s_requestEvent);
        threadWaitForExit(&s_thread);
        threadClose(&s_thread);
    }

    bool request() {
        if (!s_running || s_busy.exchange(true))
            return false;

        ueventSignal(&s_requestEvent);
        return true;
    }

    std::string status() {
        std::scoped_lock lock(s_statusMutex);
        return s_status;
    }

    u32 statusVersion() {
        return s_statusVersion;
    }

}
