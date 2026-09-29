#pragma once

#include <tesla.hpp>

// Ghi tổ hợp phím mới: giữ 2-4 nút cùng lúc khoảng 1.5 giây để lưu. Bấm riêng B rồi thả để huỷ.

class HotkeyGui : public tsl::Gui {
public:
    tsl::elm::Element* createUI() override;
    void update() override;
    bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState& touchPos,
                     HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override;

private:
    enum class State { WaitRelease, Recording, Saved };

    State m_state = State::WaitRelease;
    u64 m_candidate = 0;    ///< Tổ hợp đang giữ
    u64 m_maxHeld = 0;      ///< Tất cả nút đã giữ từ lần thả hết gần nhất (để nhận biết "chỉ bấm B")
    u32 m_stableFrames = 0;
    u32 m_savedFrames = 0;
    std::string m_message;
};
