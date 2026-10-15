#include "core/providers.hpp"

#include <algorithm>
#include <cctype>

namespace providers {

    namespace {

        // Khớp danh sách x-provider mà backend chấp nhận
        constexpr const char* SupportedProviders[] = {
            "gemini", "claude", "anthropic", "openai", "gpt", "openrouter", "openai-compatible",
        };

        // Khớp giới hạn của backend (x-api-key tối đa 512 ký tự)
        constexpr size_t MaxKeyLength = 512;
        constexpr size_t MaxBaseUrlLength = 300;

    }

    bool requiresModel(const std::string& id) {
        // Backend không có model mặc định cho hai provider này
        return id == "openrouter" || id == "openai-compatible";
    }

    bool requiresBaseUrl(const std::string& id) {
        return id == "openai-compatible";
    }

    std::string validateBaseUrl(const std::string& url) {
        if (url.rfind("https://", 0) != 0)
            return "base_url phải bắt đầu bằng https://";
        if (url.size() > MaxBaseUrlLength)
            return "base_url quá dài";
        for (unsigned char c : url) {
            if (std::isspace(c) || std::iscntrl(c) || c > 0x7E)
                return "base_url có ký tự không hợp lệ";
        }
        return "";
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
