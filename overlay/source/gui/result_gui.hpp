#pragma once

#include <tesla.hpp>

// Danh sách bản dịch gần nhất: mỗi đoạn gồm bản gốc (nhỏ, xám) và bản dịch (trắng), tự xuống dòng

class ResultGui : public tsl::Gui {
public:
    tsl::elm::Element* createUI() override;
};
