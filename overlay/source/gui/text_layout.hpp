#pragma once

#include <string>
#include <vector>

// Tách chuỗi thành nhiều dòng vừa với chiều rộng cho trước (đo bằng font thật của Tesla)

namespace text_layout {

    std::vector<std::string> wrap(const std::string& text, float fontSize, int maxWidth);

}
