#include "gui/hotkey_gui.hpp"

#include "app/hotkey.hpp"
#include "config.hpp"

namespace {

    constexpr u32 HoldFramesToSave = 90;   // ~1.5 giây ở 60 FPS
    constexpr u32 SavedFramesToClose = 60; // hiện "Đã lưu" ~1 giây rồi quay lại

}

tsl::elm::Element* HotkeyGui::createUI() {
    auto frame = new tsl::elm::OverlayFrame(config::AppName, "Đổi phím tắt");

    frame->setContent(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer* r, s32 x, s32 y, s32 w, s32 h) {
        const auto text = r->a(tsl::style::color::ColorText);
        const auto dim = r->a(tsl::style::color::ColorDescription);

        r->drawString("Phím tắt hiện tại:", false, x + 20, y + 40, 18, dim);
        r->drawString(hotkey::toGlyphs(hotkey::get()).c_str(), false, x + 20, y + 80, 32, text);
        r->drawString(hotkey::toString(hotkey::get()).c_str(), false, x + 20, y + 110, 16, dim);

        r->drawString("Giữ cùng lúc 2-4 nút để đặt phím mới.\nGiữ yên khoảng 1.5 giây để lưu.\nBấm riêng B rồi thả để huỷ.",
                      false, x + 20, y + 170, 18, text);

        // Tổ hợp đang giữ + thanh tiến trình
        s32 liveY = y + 300;
        if (m_state == State::Recording && m_candidate != 0) {
            r->drawString(hotkey::toGlyphs(m_candidate).c_str(), false, x + 20, liveY, 40, text);

            s32 barWidth = w - 40;
            r->drawRect(x + 20, liveY + 30, barWidth, 8, r->a(tsl::style::color::ColorFrame));
            if (hotkey::validate(m_candidate).empty())
                r->drawRect(x + 20, liveY + 30, barWidth * m_stableFrames / HoldFramesToSave, 8, r->a(tsl::style::color::ColorHighlight));
        }

        if (!m_message.empty())
            r->drawString(m_message.c_str(), false, x + 20, liveY + 80, 20, text);
    }));

    return frame;
}

void HotkeyGui::update() {
    if (m_state == State::Saved && ++m_savedFrames >= SavedFramesToClose)
        tsl::goBack();
}

bool HotkeyGui::handleInput(u64 keysDown, u64 keysHeld, const HidTouchState&, HidAnalogStickState, HidAnalogStickState) {
    // Tesla gọi riêng một lần với keysHeld=0 khi vừa nhấn B: bỏ qua frame đó
    if (keysHeld == 0 && keysDown == HidNpadButton_B)
        return true;

    const u64 held = keysHeld & hotkey::allowedMask();

    switch (m_state) {
        case State::WaitRelease:
            // Chờ thả hết nút (vd nút A vừa bấm để mở màn hình này)
            if (held == 0)
                m_state = State::Recording;
            break;

        case State::Recording:
            if (held == 0) {
                if (m_maxHeld == HidNpadButton_B) {
                    tsl::goBack();
                    return true;
                }
                m_maxHeld = 0;
            }
            m_maxHeld |= held;

            if (held != m_candidate) {
                m_candidate = held;
                m_stableFrames = 0;
                m_message = held != 0 ? hotkey::validate(held) : "";
                break;
            }

            if (held != 0 && hotkey::validate(held).empty() && ++m_stableFrames >= HoldFramesToSave) {
                bool saved = hotkey::save(held);
                m_message = saved ? "Đã lưu: " + hotkey::toString(held)
                                  : "Không ghi được config.ini (chỉ dùng tạm)";
                m_state = State::Saved;
            }
            break;

        case State::Saved:
            break;
    }

    // Nuốt mọi input để B/A không kích hoạt hành vi mặc định của Tesla
    return true;
}
