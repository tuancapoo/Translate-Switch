// Chỉ định nghĩa TESLA_INIT_IMPL ở đúng một file (file này)
#define TESLA_INIT_IMPL
#include <tesla.hpp>

#include "app/hotkey.hpp"
#include "app/translate_service.hpp"
#include "config.hpp"
#include "core/capture.hpp"
#include "core/http.hpp"
#include "gui/box_gui.hpp"
#include "gui/main_gui.hpp"

class TranslateOverlay : public tsl::Overlay {
public:
    void initServices() override {
        capture::init();
        http::init();
        hotkey::load();
        translate_service::start();
    }

    void exitServices() override {
        translate_service::stop();
        http::exit();
        capture::exit();
    }

    // Phím tắt chụp & dịch khi overlay đang ẩn
    void onHiddenInput(u64 keysDown, u64 keysHeld) override {
        const u64 combo = hotkey::get();
        if ((keysHeld & combo) == combo && (keysDown & combo))
            translate_service::request();
    }

    // Overlay được bật tự động khi dịch xong -> hiện box bản dịch
    void onShow() override {
        if (translate_service::takeShowRequest())
            presentTranslationBoxes();
    }

    // Box đã bị ẩn -> gỡ ra để lần mở sau về menu, và trả lại RAM của framebuffer fullscreen
    void onHidden() override {
        if (BoxGui::isActive())
            tsl::goBack();
    }

    std::unique_ptr<tsl::Gui> loadInitialGui() override {
        return initially<MainGui>();
    }
};

int main(int argc, char** argv) {
    // LaunchFlags::None: bấm B chỉ ẩn overlay chứ không thoát, để phím tắt vẫn hoạt động
    return tsl::loop<TranslateOverlay, tsl::impl::LaunchFlags::None>(argc, argv);
}
