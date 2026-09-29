#include "app/hotkey.hpp"

#include "config.hpp"
#include "core/settings.hpp"

#include <tesla.hpp>
#include <atomic>

namespace hotkey {

    namespace {

        std::atomic<u64> s_keys = config::DefaultHotkey;

        constexpr int MinButtons = 2;
        constexpr int MaxButtons = 4;

    }

    u64 allowedMask() {
        u64 mask = 0;
        for (const auto& info : tsl::impl::KEYS_INFO)
            mask |= info.key;
        return mask;
    }

    std::string validate(u64 keys) {
        int count = __builtin_popcountll(keys);
        if (count < MinButtons)
            return "Cần ít nhất " + std::to_string(MinButtons) + " nút";
        if (count > MaxButtons)
            return "Tối đa " + std::to_string(MaxButtons) + " nút";
        if ((keys & tsl::cfg::launchCombo) == tsl::cfg::launchCombo)
            return "Trùng tổ hợp mở menu overlay";
        return "";
    }

    void load() {
        u64 keys = tsl::hlp::comboStringToKeys(settings::readValue(config::SettingsPath, "hotkey"));
        s_keys = validate(keys).empty() ? keys : config::DefaultHotkey;
    }

    u64 get() {
        return s_keys;
    }

    bool save(u64 keys) {
        s_keys = keys;
        return settings::writeValue(config::SettingsPath, "hotkey", toString(keys));
    }

    std::string toString(u64 keys) {
        return tsl::hlp::keysToComboString(keys);
    }

    std::string toGlyphs(u64 keys) {
        std::string result;
        for (const auto& info : tsl::impl::KEYS_INFO) {
            if (keys & info.key) {
                if (!result.empty())
                    result += " + ";
                result += info.glyph;
            }
        }
        return result;
    }

}
