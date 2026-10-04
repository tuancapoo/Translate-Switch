#pragma once

#include <tesla.hpp>

// Ô vuông nhỏ góc trên bên trái báo tiến trình chụp & dịch:
// vàng = đang dịch, xanh = dịch xong, đỏ = lỗi (xanh/đỏ tự tắt sau vài giây).

namespace status_indicator {

    /// Vẽ ô vuông nếu đang có trạng thái cần báo. Trả về true khi còn hiện (cần vẽ lại để tự tắt đúng lúc).
    bool draw(tsl::gfx::Renderer* renderer);

}
