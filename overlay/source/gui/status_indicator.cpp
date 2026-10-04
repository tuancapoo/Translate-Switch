#include "gui/status_indicator.hpp"

#include "app/translate_service.hpp"

#include <algorithm>

namespace status_indicator {

    namespace {

        constexpr tsl::Color WorkingColor = { 0xF, 0xC, 0x0, 0xF };  // vàng
        constexpr tsl::Color DoneColor    = { 0x2, 0xD, 0x4, 0xF };  // xanh lá
        constexpr tsl::Color FailedColor  = { 0xF, 0x2, 0x2, 0xF };  // đỏ
        constexpr tsl::Color BorderColor  = { 0x0, 0x0, 0x0, 0xA };  // viền tối cho dễ thấy trên nền sáng

        // Thời gian giữ ô xanh / đỏ sau khi dịch xong
        constexpr u64 DoneVisibleNs   = 3'000'000'000;
        constexpr u64 FailedVisibleNs = 5'000'000'000;

        // Kích thước theo màn hình 1280x720
        constexpr float Margin = 12.0F;
        constexpr float Size   = 14.0F;
        constexpr float Border = 2.0F;

        bool expired(u64 visibleNs) {
            return armTicksToNs(armGetSystemTick() - translate_service::phaseTick()) > visibleNs;
        }

    }

    bool draw(tsl::gfx::Renderer* renderer) {
        tsl::Color color = WorkingColor;
        switch (translate_service::phase()) {
            case translate_service::Phase::Idle:
                return false;
            case translate_service::Phase::Working:
                break;
            case translate_service::Phase::Done:
                if (expired(DoneVisibleNs))
                    return false;
                color = DoneColor;
                break;
            case translate_service::Phase::Failed:
                if (expired(FailedVisibleNs))
                    return false;
                color = FailedColor;
                break;
        }

        // Framebuffer có thể là panel 448x720 hoặc fullscreen độ phân giải thấp hơn 720p
        const float scale = tsl::cfg::FramebufferHeight / 720.0F;
        const s32 margin = static_cast<s32>(Margin * scale);
        const s32 size   = std::max(4, static_cast<s32>(Size * scale));
        const s32 border = std::max(1, static_cast<s32>(Border * scale));

        renderer->drawRect(margin - border, margin - border, size + border * 2, size + border * 2, BorderColor);
        renderer->drawRect(margin, margin, size, size, color);
        return true;
    }

}
