#include "gui/main_gui.hpp"

#include "app/hotkey.hpp"
#include "app/translate_service.hpp"
#include "config.hpp"
#include "gui/box_gui.hpp"
#include "gui/hotkey_gui.hpp"
#include "gui/result_gui.hpp"

namespace {

    std::string resultsLabel() {
        return "Xem bản dịch (" + std::to_string(translate_service::translation().lines.size()) + ")";
    }

    std::string hotkeyLabel() {
        return "Phím tắt: " + hotkey::toGlyphs(hotkey::get());
    }

}

tsl::elm::Element* MainGui::createUI() {
    auto frame = new tsl::elm::OverlayFrame(config::AppName, config::AppVersion);
    auto list  = new tsl::elm::List();

    list->addItem(new tsl::elm::CategoryHeader("Dịch màn hình"));

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

    auto boxesItem = new tsl::elm::ListItem("Hiện box trên màn hình");
    boxesItem->setClickListener([](u64 keys) {
        if (!(keys & HidNpadButton_A))
            return false;
        if (!translate_service::translation().lines.empty())
            presentTranslationBoxes();
        return true;
    });
    list->addItem(boxesItem);

    list->addItem(new tsl::elm::CategoryHeader("Cài đặt"));

    m_hotkeyItem = new tsl::elm::ListItem(hotkeyLabel(), "Đổi");
    m_shownHotkey = hotkey::get();
    m_hotkeyItem->setClickListener([](u64 keys) {
        if (!(keys & HidNpadButton_A))
            return false;
        tsl::changeTo<HotkeyGui>();
        return true;
    });
    list->addItem(m_hotkeyItem);

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

    // Quay về từ màn hình đổi phím tắt
    if (hotkey::get() != m_shownHotkey) {
        m_shownHotkey = hotkey::get();
        m_hotkeyItem->setText(hotkeyLabel());
    }

    // Dịch xong trong lúc đang mở menu -> chuyển thẳng sang box
    if (translate_service::takeShowRequest())
        presentTranslationBoxes();
}
