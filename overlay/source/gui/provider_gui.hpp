#pragma once

#include <tesla.hpp>

// Chọn provider. Chọn xong tự quay lại menu; đổi provider sẽ đặt lại model về mặc định.

class ProviderGui : public tsl::Gui {
public:
    tsl::elm::Element* createUI() override;
    void update() override;

private:
    // Quay lại ở update() thay vì ngay trong click listener (listener thuộc về Gui sắp bị huỷ)
    bool m_done = false;
};
