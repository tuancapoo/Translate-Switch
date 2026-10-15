#pragma once

#include <tesla.hpp>

// Chỉnh độ trong suốt của box bản dịch: kéo thanh trượt (trái/phải) và xem trước ngay.
// Giá trị được lưu vào config.ini khi thoát màn hình này (bấm B).

class OpacityGui : public tsl::Gui {
public:
    ~OpacityGui() override;

    tsl::elm::Element* createUI() override;
};
