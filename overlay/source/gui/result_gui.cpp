#include "gui/result_gui.hpp"

#include "app/translate_service.hpp"
#include "config.hpp"
#include "gui/text_layout.hpp"

namespace {

    constexpr float SourceFontSize = 16;
    constexpr float TranslationFontSize = 20;
    constexpr int TextWidth = 360;   // chiều rộng vùng chữ trong panel 448px
    constexpr int PaddingX = 20;
    constexpr int PaddingY = 12;

    /// Một đoạn bản dịch đã tính sẵn xuống dòng, vẽ bằng CustomDrawer
    tsl::elm::Element* makeEntry(const translate_client::TextLine& line, u16& outHeight) {
        auto source = text_layout::wrap(line.source, SourceFontSize, TextWidth);
        auto translation = text_layout::wrap(line.translation, TranslationFontSize, TextWidth);

        outHeight = static_cast<u16>(PaddingY * 2 + source.size() * SourceFontSize + 6 + translation.size() * TranslationFontSize);

        return new tsl::elm::CustomDrawer([source = std::move(source), translation = std::move(translation)](
                                              tsl::gfx::Renderer* renderer, s32 x, s32 y, s32 w, s32 h) {
            s32 cursorY = y + PaddingY + SourceFontSize;
            for (const auto& text : source) {
                renderer->drawString(text.c_str(), false, x + PaddingX, cursorY, SourceFontSize, renderer->a(tsl::style::color::ColorDescription));
                cursorY += SourceFontSize;
            }

            cursorY += 6;
            for (const auto& text : translation) {
                renderer->drawString(text.c_str(), false, x + PaddingX, cursorY, TranslationFontSize, renderer->a(tsl::style::color::ColorText));
                cursorY += TranslationFontSize;
            }

            renderer->drawRect(x, y + h - 1, w, 1, renderer->a(tsl::style::color::ColorFrame));
        });
    }

}

tsl::elm::Element* ResultGui::createUI() {
    auto frame = new tsl::elm::OverlayFrame(config::AppName, "Bản dịch");
    auto list = new tsl::elm::List();

    auto translation = translate_service::translation();
    if (translation.lines.empty()) {
        list->addItem(new tsl::elm::ListItem("Chưa có bản dịch"));
    } else {
        for (const auto& line : translation.lines) {
            u16 height = 0;
            auto entry = makeEntry(line, height);
            list->addItem(entry, height);
        }
    }

    frame->setContent(list);
    return frame;
}
