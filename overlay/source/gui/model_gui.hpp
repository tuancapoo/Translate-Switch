#pragma once

#include <tesla.hpp>

// Chọn model cho provider đang dùng. Provider không có danh sách sẵn (OpenRouter, API chuẩn OpenAI)
// thì hướng dẫn đặt model trong config.ini.

class ModelGui : public tsl::Gui {
public:
    tsl::elm::Element* createUI() override;
    void update() override;

private:
    bool m_done = false;
};
