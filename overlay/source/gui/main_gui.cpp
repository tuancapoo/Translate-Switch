#include "gui/main_gui.hpp"

#include "app/capture_service.hpp"
#include "config.hpp"

tsl::elm::Element* MainGui::createUI() {
    auto frame = new tsl::elm::OverlayFrame(config::AppName, config::AppVersion);
    auto list  = new tsl::elm::List();

    list->addItem(new tsl::elm::CategoryHeader(std::string("Chụp màn hình · phím tắt ") + config::CaptureHotkeyName));

    auto captureItem = new tsl::elm::ListItem("Chụp & lưu ra SD");
    captureItem->setClickListener([](u64 keys) {
        if (!(keys & HidNpadButton_A))
            return false;
        capture_service::request();
        return true;
    });
    list->addItem(captureItem);

    m_statusItem = new tsl::elm::ListItem(capture_service::status());
    m_shownStatusVersion = capture_service::statusVersion();
    list->addItem(m_statusItem);

    list->addItem(new tsl::elm::CategoryHeader("Thông tin"));
    list->addItem(new tsl::elm::ListItem(tsl::gfx::Renderer::s_customFontStatus));

    // Overlay chạy nền để bắt phím tắt; B chỉ ẩn, muốn về menu Ultrahand thì thoát ở đây
    auto exitItem = new tsl::elm::ListItem("Thoát overlay");
    exitItem->setClickListener([](u64 keys) {
        if (!(keys & HidNpadButton_A))
            return false;
        tsl::Overlay::get()->close();
        return true;
    });
    list->addItem(exitItem);

    frame->setContent(list);
    return frame;
}

void MainGui::update() {
    // Status được thread nền cập nhật, chỉ đổi text khi có thay đổi
    u32 version = capture_service::statusVersion();
    if (version != m_shownStatusVersion) {
        m_shownStatusVersion = version;
        m_statusItem->setText(capture_service::status());
    }
}
