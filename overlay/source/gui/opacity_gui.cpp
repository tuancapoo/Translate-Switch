#include "gui/opacity_gui.hpp"

#include "app/box_style.hpp"
#include "config.hpp"

namespace {

    constexpr u16 PreviewHeight = 250;

    constexpr tsl::Color LightBackground = { 0xD, 0xD, 0xD, 0xF };
    constexpr tsl::Color DarkBackground  = { 0x3, 0x5, 0x8, 0xF };
    constexpr tsl::Color TextColor       = { 0xF, 0xF, 0xF, 0xF };

}

OpacityGui::~OpacityGui() {
    // Lưu khi rời màn hình (không ghi file mỗi lần kéo thanh trượt)
    box_style::save();
}

tsl::elm::Element* OpacityGui::createUI() {
    auto frame = new tsl::elm::OverlayFrame(config::AppName, "Độ trong suốt box");
    auto list = new tsl::elm::List();

    auto preview = new tsl::elm::CustomDrawer([](tsl::gfx::Renderer* r, s32 x, s32 y, s32 w, s32 h) {
        r->drawString(("Nền box: đặc " + std::to_string(box_style::opacityPercent()) + "%").c_str(),
                      false, x + 20, y + 36, 24, r->a(tsl::style::color::ColorText));
        r->drawString("Trái: trong suốt hơn · Phải: đặc hơn", false, x + 20, y + 64, 16,
                      r->a(tsl::style::color::ColorDescription));

        // Nền hai màu sáng/tối để thấy rõ box trong suốt ra sao
        const s32 stripY = y + 90;
        const s32 stripH = h - 100;
        r->drawRect(x + 20, stripY, (w - 40) / 2, stripH, r->a(LightBackground));
        r->drawRect(x + 20 + (w - 40) / 2, stripY, (w - 40) - (w - 40) / 2, stripH, r->a(DarkBackground));

        const tsl::Color boxColor(0x0, 0x0, 0x0, box_style::alpha());
        const s32 boxW = w - 80;
        const s32 boxH = 46;
        const s32 boxX = x + 40;
        const s32 boxY = stripY + (stripH - boxH) / 2;
        r->drawRect(boxX, boxY, boxW, boxH, r->a(boxColor));
        r->drawString("Xin chào, đây là bản dịch", false, boxX + 8, boxY + 30, 22, r->a(TextColor));
    });
    list->addItem(preview, PreviewHeight);

    auto slider = new tsl::elm::TrackBar("");
    slider->setProgress(static_cast<u8>(box_style::opacityPercent()));
    slider->setValueChangedListener([](u8 value) {
        box_style::setOpacityPercent(value);
    });
    list->addItem(slider);

    frame->setContent(list);
    return frame;
}
