#include "core/providers.hpp"

#include <algorithm>
#include <cctype>

namespace providers {

    namespace {

        // Hiện chỉ cho chọn Gemini
        constexpr const char* SupportedProviders[] = { "gemini" };

        // Khớp giới hạn của backend (x-api-key tối đa 512 ký tự)
        constexpr size_t MaxKeyLength = 512;

    }

    bool isSupported(const std::string& id) {
        return std::any_of(std::begin(SupportedProviders), std::end(SupportedProviders),
                           [&](const char* supported) { return id == supported; });
    }

    std::string supportedList() {
        std::string list;
        for (const char* id : SupportedProviders) {
            if (!list.empty())
                list += ", ";
            list += id;
        }
        return list;
    }

    std::string validateKey(const std::string& key) {
        if (key.size() > MaxKeyLength)
            return "api_key quá dài";

        // Cấm khoảng trắng và ký tự điều khiển: key được đưa vào HTTP header
        for (unsigned char c : key) {
            if (std::isspace(c) || std::iscntrl(c) || c > 0x7E)
                return "api_key có ký tự không hợp lệ";
        }
        return "";
    }

    std::string maskKey(const std::string& key) {
        if (key.empty())
            return "";
        if (key.size() <= 8)
            return "****";
        return "****" + key.substr(key.size() - 4);
    }

}
