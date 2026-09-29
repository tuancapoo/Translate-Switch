#include "app/translate_service.hpp"

#include "config.hpp"
#include "core/capture.hpp"
#include "core/http.hpp"
#include "core/settings.hpp"
#include "core/storage.hpp"
#include "core/util.hpp"

#include <tesla.hpp>
#include <atomic>
#include <cstdio>
#include <mutex>

namespace translate_service {

    namespace {

        // curl + TLS handshake cần stack lớn hơn mặc định
        constexpr size_t ThreadStackSize = 0x20000;

        enum class Job { Translate, Diagnose };

        Thread s_thread;
        UEvent s_requestEvent;
        std::atomic<bool> s_running = false;
        std::atomic<bool> s_busy = false;
        std::atomic<Job> s_job = Job::Translate;

        std::mutex s_mutex;
        std::string s_status = "Sẵn sàng";
        translate_client::Translation s_translation;
        std::atomic<u32> s_statusVersion = 0;
        std::atomic<u32> s_translationVersion = 0;

        void setStatus(std::string text) {
            {
                std::scoped_lock lock(s_mutex);
                s_status = std::move(text);
            }
            s_statusVersion++;
        }

        std::string formatSeconds(u64 startTick) {
            char buffer[16];
            snprintf(buffer, sizeof(buffer), "%.1fs", armTicksToNs(armGetSystemTick() - startTick) / 1e9);
            return buffer;
        }

        void runPipeline() {
            // Đọc lại mỗi lần để sửa config.ini không cần khởi động lại overlay
            settings::Settings userSettings;
            std::string settingsError = settings::load(config::SettingsPath, userSettings);
            if (!settingsError.empty()) {
                setStatus(settingsError);
                return;
            }

            setStatus("Đang chụp...");
            capture::Frame frame;
            Result rc = capture::captureJpeg(frame);
            if (R_FAILED(rc)) {
                setStatus("Lỗi chụp: " + util::resultToString(rc));
                return;
            }

            // Lưu lại ảnh vừa chụp để debug (không bắt buộc thành công)
            storage::writeFile(config::CapturePath, frame.data, frame.size);

            setStatus("Đang dịch...");
            u64 startTick = armGetSystemTick();
            translate_client::Translation result;
            std::string error = translate_client::translate(userSettings, frame.data, frame.size, result);
            if (!error.empty()) {
                setStatus(error);
                return;
            }

            std::string summary = "✓ " + std::to_string(result.lines.size()) + " dòng · " + formatSeconds(startTick);
            if (!result.provider.empty())
                summary += " · " + result.provider;

            {
                std::scoped_lock lock(s_mutex);
                s_translation = std::move(result);
            }
            s_translationVersion++;
            setStatus(summary);
        }

        void runDiagnose() {
            settings::Settings userSettings;
            std::string settingsError = settings::load(config::SettingsPath, userSettings);

            setStatus("Đang kiểm tra mạng...");
            std::string report = http::diagnose(settingsError.empty() ? userSettings.url : "");
            setStatus(settingsError.empty() ? report : report + " · " + settingsError);
        }

        void threadMain(void*) {
            // libnx mở service DNS (sfdnsres) theo từng request qua sm, nhưng Tesla đã đóng sm sau initServices.
            // Giữ một sm session suốt đời thread, nếu không getaddrinfo trả EAI_SYSTEM (11).
            smInitialize();
            tsl::hlp::ScopeGuard smGuard([] { smExit(); });

            while (true) {
                waitSingle(waiterForUEvent(&s_requestEvent), UINT64_MAX);
                if (!s_running)
                    break;

                if (s_job == Job::Diagnose)
                    runDiagnose();
                else
                    runPipeline();
                s_busy = false;
            }
        }

        bool submit(Job job) {
            if (!s_running || s_busy.exchange(true))
                return false;

            s_job = job;
            ueventSignal(&s_requestEvent);
            return true;
        }

    }

    void start() {
        if (s_running)
            return;

        ueventCreate(&s_requestEvent, true);
        s_running = true;
        threadCreate(&s_thread, threadMain, nullptr, nullptr, ThreadStackSize, 0x2C, -2);
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
        return submit(Job::Translate);
    }

    bool requestDiagnose() {
        return submit(Job::Diagnose);
    }

    std::string status() {
        std::scoped_lock lock(s_mutex);
        return s_status;
    }

    u32 statusVersion() {
        return s_statusVersion;
    }

    translate_client::Translation translation() {
        std::scoped_lock lock(s_mutex);
        return s_translation;
    }

    u32 translationVersion() {
        return s_translationVersion;
    }

}
