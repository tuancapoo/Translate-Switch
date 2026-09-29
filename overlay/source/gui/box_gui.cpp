#include "gui/box_gui.hpp"

#include "app/translate_service.hpp"
#include "gui/text_layout.hpp"

#include <algorithm>

namespace {

    // Toạ độ từ server theo ảnh chụp 1280x720
    constexpr float SourceWidth = 1280.0F;

    constexpr tsl::Color BoxColor  = { 0x0, 0x0, 0x0, 0xB };  // đen, alpha 11/15 ≈ 70%
    constexpr tsl::Color TextColor = { 0xF, 0xF, 0xF, 0xF };  // trắng

    constexpr float MaxFontSize = 26.0F;
    constexpr float MinFontSize = 12.0F;
    constexpr int Padding = 4;
    constexpr int MinBoxWidth = 60;

    // Bỏ qua input vài frame đầu: phím bấm lúc chơi game (vd L+R+A) còn dồn lại khi overlay vừa hiện
    constexpr u32 InputGraceFrames = 15;

    bool s_active = false;

    struct Box {
        s32 x, y, w, h;
        float fontSize;
        std::vector<std::string> lines;
    };

    /// Chọn cỡ chữ lớn nhất mà bản dịch vừa box; nếu cỡ nhỏ nhất vẫn không vừa thì kéo dài box xuống dưới
    Box layoutBox(const translate_client::TextLine& line, float scale, s32 screenW, s32 screenH) {
        Box box;
        box.x = static_cast<s32>(line.x * scale) - Padding;
        box.y = static_cast<s32>(line.y * scale) - Padding;
        box.w = std::max(static_cast<s32>(line.w * scale) + Padding * 2, static_cast<s32>(MinBoxWidth * scale));
        box.h = static_cast<s32>(line.h * scale) + Padding * 2;

        box.x = std::clamp(box.x, 0, std::max(0, screenW - box.w));
        box.y = std::clamp(box.y, 0, std::max(0, screenH - 1));

        const int textWidth = std::max(1, box.w - Padding * 2);
        const float maxSize = std::max(MinFontSize, MaxFontSize * scale);
        const float minSize = std::max(8.0F, MinFontSize * scale);

        for (float size = maxSize; size >= minSize; size -= 1.0F) {
            box.fontSize = size;
            box.lines = text_layout::wrap(line.translation, size, textWidth);
            if (static_cast<s32>(box.lines.size() * size) + Padding * 2 <= box.h)
                return box;
        }

        box.h = std::min(static_cast<s32>(box.lines.size() * box.fontSize) + Padding * 2, screenH - box.y);
        return box;
    }

    class BoxesElement : public tsl::elm::Element {
    public:
        explicit BoxesElement(BoxGui* gui) : m_gui(gui), m_lines(translate_service::translation().lines) {}

        void draw(tsl::gfx::Renderer* renderer) override {
            // Layout fullscreen chỉ áp dụng từ frame kế tiếp (và có thể fallback độ phân giải thấp hơn),
            // nên tính box theo kích thước framebuffer thực tế lúc vẽ
            if (m_builtForWidth != tsl::cfg::FramebufferWidth) {
                m_builtForWidth = tsl::cfg::FramebufferWidth;
                const float scale = tsl::cfg::FramebufferWidth / SourceWidth;
                m_boxes.clear();
                for (const auto& line : m_lines)
                    m_boxes.push_back(layoutBox(line, scale, tsl::cfg::FramebufferWidth, tsl::cfg::FramebufferHeight));
            }

            renderer->clearScreen();
            if (!tsl::gfx::Renderer::isFullscreen()) {
                // Frame đầu có thể còn là panel (layout áp dụng ở frame sau): chưa báo lỗi vội
                if (++m_panelFrames < 3)
                    return;
                // Không tạo được framebuffer fullscreen (thiếu RAM): báo trong panel
                renderer->fillScreen(renderer->a(tsl::style::color::ColorFrameBackground));
                renderer->drawString("Không đủ RAM để vẽ toàn màn hình.\nBấm B để quay lại.", false, 20, 60, 20, renderer->a(TextColor));
                return;
            }

            for (const auto& box : m_boxes) {
                renderer->drawRect(box.x, box.y, box.w, box.h, renderer->a(BoxColor));

                s32 baseline = box.y + Padding + static_cast<s32>(box.fontSize);
                for (const auto& text : box.lines) {
                    renderer->drawString(text.c_str(), false, box.x + Padding, baseline, box.fontSize, renderer->a(TextColor));
                    baseline += static_cast<s32>(box.fontSize);
                }
            }
        }

        void layout(u16, u16, u16, u16) override {}

        bool onTouch(tsl::elm::TouchEvent event, s32, s32, s32, s32, s32, s32) override {
            if (event == tsl::elm::TouchEvent::Release)
                m_gui->dismiss();
            return true;
        }

    private:
        BoxGui* m_gui;
        std::vector<translate_client::TextLine> m_lines;
        std::vector<Box> m_boxes;
        u16 m_builtForWidth = 0;
        u32 m_panelFrames = 0;
    };

}

BoxGui::BoxGui() {
    s_active = true;
    tsl::gfx::Renderer::requestLayout(tsl::gfx::Renderer::Layout::Fullscreen);
}

BoxGui::~BoxGui() {
    s_active = false;
    tsl::gfx::Renderer::requestLayout(tsl::gfx::Renderer::Layout::Panel);
}

bool BoxGui::isActive() {
    return s_active;
}

tsl::elm::Element* BoxGui::createUI() {
    return new BoxesElement(this);
}

void BoxGui::update() {
    m_frames++;
}

bool BoxGui::handleInput(u64 keysDown, u64, const HidTouchState&, HidAnalogStickState, HidAnalogStickState) {
    constexpr u64 dismissKeys = HidNpadButton_A | HidNpadButton_B | HidNpadButton_X | HidNpadButton_Y |
                                HidNpadButton_L | HidNpadButton_R | HidNpadButton_ZL | HidNpadButton_ZR |
                                HidNpadButton_Plus | HidNpadButton_Minus;

    if (m_frames > InputGraceFrames && (keysDown & dismissKeys))
        this->dismiss();
    return true;
}

void BoxGui::dismiss() {
    if (m_frames <= InputGraceFrames || m_dismissed)
        return;
    m_dismissed = true;
    tsl::Overlay::get()->hide();
}

void presentTranslationBoxes() {
    if (BoxGui::isActive())
        tsl::goBack();
    tsl::changeTo<BoxGui>();
}
