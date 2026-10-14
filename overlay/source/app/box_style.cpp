#include "app/box_style.hpp"

#include "config.hpp"
#include "core/settings.hpp"

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <string>

namespace box_style {

    namespace {

        std::atomic<int> s_percent = config::DefaultBoxOpacity;
        std::atomic<u32> s_version = 0;

        constexpr const char* Key = "box_opacity";

    }

    void load() {
        std::string text = settings::readValue(config::SettingsPath, Key);
        if (text.empty())
            return;

        char* end = nullptr;
        long value = std::strtol(text.c_str(), &end, 10);
        if (end != text.c_str() && *end == '\0')
            s_percent = static_cast<int>(std::clamp(value, 0L, 100L));
    }

    int opacityPercent() {
        return s_percent;
    }

    void setOpacityPercent(int percent) {
        percent = std::clamp(percent, 0, 100);
        if (s_percent.exchange(percent) != percent)
            s_version++;
    }

    bool save() {
        return settings::writeValue(config::SettingsPath, Key, std::to_string(s_percent.load()));
    }

    u8 alpha() {
        // 4 bit alpha: 16 mức; làm tròn để 70% ra 11/15
        return static_cast<u8>((s_percent * 15 + 50) / 100);
    }

    u32 version() {
        return s_version;
    }

}
