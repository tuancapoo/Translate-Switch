#include "gui/main_gui.hpp"

#include "app/translate_service.hpp"
#include "config.hpp"
#include "gui/result_gui.hpp"

namespace {

    std::string resultsLabel() {
        return "Xem bản dịch (" + std::to_string(translate_service::translation().lines.size()) + ")";
    }

}

tsl::elm::Element* MainGui::createUI() {
    auto frame = new tsl::elm::OverlayFrame(config::AppName, config::AppVersion);
    auto list  = new tsl::elm::List();

    list->addItem(new tsl::elm::CategoryHeader(std::string("Dịch màn hình · phím tắt ") + config::CaptureHotkeyName));

    auto translateItem = new tsl::elm::ListItem("Chụp & dịch");
    translateItem->setClickListener([](u64 keys) {
        if (!(keys & HidNpadButton_A))
            return false;
        translate_service::request();
        return true;
    });
    list->addItem(translateItem);

    m_statusItem = new tsl::elm::ListItem(translate_service::status());
    m_shownStatusVersion = translate_service::statusVersion();
    list->addItem(m_statusItem);

    m_resultsItem = new tsl::elm::ListItem(resultsLabel());
    m_shownTranslationVersion = translate_service::translationVersion();
    m_resultsItem->setClickListener([](u64 keys) {
        if (!(keys & HidNpadButton_A))
            return false;
        tsl::changeTo<ResultGui>();
        return true;
    });
    list->addItem(m_resultsItem);

    list->addItem(new tsl::elm::CategoryHeader("Thông tin"));

    auto diagnoseItem = new tsl::elm::ListItem("Kiểm tra mạng");
    diagnoseItem->setClickListener([](u64 keys) {
        if (!(keys & HidNpadButton_A))
            return false;
        translate_service::requestDiagnose();
        return true;
    });
    list->addItem(diagnoseItem);

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
    // Status và bản dịch được thread nền cập nhật, chỉ đổi text khi có thay đổi
    u32 version = translate_service::statusVersion();
    if (version != m_shownStatusVersion) {
        m_shownStatusVersion = version;
        m_statusItem->setText(translate_service::status());
    }

    u32 translationVersion = translate_service::translationVersion();
    if (translationVersion != m_shownTranslationVersion) {
        m_shownTranslationVersion = translationVersion;
        m_resultsItem->setText(resultsLabel());
    }
}
