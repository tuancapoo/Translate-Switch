// Chỉ định nghĩa TESLA_INIT_IMPL ở đúng một file (file này)
#define TESLA_INIT_IMPL
#include <tesla.hpp>

#include "app/translate_service.hpp"
#include "config.hpp"
#include "core/capture.hpp"
#include "core/http.hpp"
#include "gui/main_gui.hpp"

class TranslateOverlay : public tsl::Overlay {
public:
    void initServices() override {
        capture::init();
        http::init();
        translate_service::start();
    }

    void exitServices() override {
        translate_service::stop();
        http::exit();
        capture::exit();
    }

    // Phím tắt chụp & dịch khi overlay đang ẩn
    void onHiddenInput(u64 keysDown, u64 keysHeld) override {
        constexpr u64 combo = config::CaptureHotkey;
        if ((keysHeld & combo) == combo && (keysDown & combo))
            translate_service::request();
    }

    std::unique_ptr<tsl::Gui> loadInitialGui() override {
        return initially<MainGui>();
    }
};

int main(int argc, char** argv) {
    // LaunchFlags::None: bấm B chỉ ẩn overlay chứ không thoát, để phím tắt vẫn hoạt động
    return tsl::loop<TranslateOverlay, tsl::impl::LaunchFlags::None>(argc, argv);
}
