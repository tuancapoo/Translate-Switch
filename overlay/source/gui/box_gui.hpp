#pragma once

#include <tesla.hpp>

// Vẽ bản dịch toàn màn hình: mỗi đoạn chữ là một box đen trong suốt ~70% đặt đúng vị trí chữ gốc,
// chữ trắng tự xuống dòng và thu nhỏ cho vừa box. Chạm màn hình hoặc bấm nút bất kỳ để ẩn.

class BoxGui : public tsl::Gui {
public:
    BoxGui();
    ~BoxGui() override;

    tsl::elm::Element* createUI() override;
    void update() override;
    bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState& touchPos,
                     HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override;

    /// Có đang nằm trên gui stack không (để main.cpp biết cần gỡ ra khi mở lại overlay)
    static bool isActive();

    /// Ẩn các box và ẩn overlay
    void dismiss();

private:
    u32 m_frames = 0;
    bool m_dismissed = false;
};

/// Hiện bản dịch mới nhất dạng box (thay box cũ nếu đang hiện)
void presentTranslationBoxes();
