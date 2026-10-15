#include "app/provider_choice.hpp"

#include "config.hpp"
#include "core/settings.hpp"

#include <atomic>

namespace provider_choice {

    namespace {

        std::atomic<u32> s_version = 0;

        const std::vector<Provider> Providers = {
            { "gemini", "Gemini (Google)" },
            { "claude", "Claude (Anthropic)" },
            { "openai", "ChatGPT (OpenAI)" },
            { "openrouter", "OpenRouter" },
            { "openai-compatible", "API chuẩn OpenAI" },
        };

        // Chỉ liệt kê model đã biết là tồn tại; model khác đặt tay trong config.ini (model=...)
        const std::vector<Model> GeminiModels = {
            { "gemini-3.5-flash-lite", "mặc định" },
            { "gemini-3.1-flash-lite", "" },
            { "gemini-3-flash-preview", "preview" },
        };
        const std::vector<Model> ClaudeModels = {
            { "claude-opus-5-5", "mặc định, chậm" },
            { "claude-sonnet-5-5", "" },
            { "claude-haiku-4-5", "nhanh" },
        };
        const std::vector<Model> OpenAiModels = {
            { "gpt-6-luna", "mặc định" },
        };
        const std::vector<Model> NoModels;

        std::string canonical(const std::string& id) {
            if (id == "anthropic")
                return "claude";
            if (id == "gpt")
                return "openai";
            return id;
        }

    }

    const std::vector<Provider>& providers() {
        return Providers;
    }

    const std::vector<Model>& presets(const std::string& providerId) {
        const std::string id = canonical(providerId);
        if (id == "gemini")
            return GeminiModels;
        if (id == "claude")
            return ClaudeModels;
        if (id == "openai")
            return OpenAiModels;
        return NoModels;
    }

    std::string defaultModel(const std::string& providerId) {
        const std::string id = canonical(providerId);
        if (id == "gemini")
            return "gemini-3.5-flash-lite";
        if (id == "claude")
            return "claude-opus-5-5";
        if (id == "openai")
            return "gpt-6-luna";
        return "";
    }

    std::string label(const std::string& providerId) {
        const std::string id = canonical(providerId);
        for (const auto& provider : Providers) {
            if (id == provider.id)
                return provider.label;
        }
        return providerId;
    }

    std::string currentProvider() {
        std::string id = settings::readValue(config::SettingsPath, "provider");
        return id.empty() ? "gemini" : canonical(id);
    }

    std::string currentModel() {
        return settings::readValue(config::SettingsPath, "model");
    }

    bool setProvider(const std::string& id) {
        const std::string previous = currentProvider();
        bool ok = settings::writeValue(config::SettingsPath, "provider", id);
        // Model của provider này không dùng được cho provider khác: đặt lại về mặc định
        if (ok && id != previous)
            ok = settings::writeValue(config::SettingsPath, "model", "");
        s_version++;
        return ok;
    }

    bool setModel(const std::string& id) {
        bool ok = settings::writeValue(config::SettingsPath, "model", id);
        s_version++;
        return ok;
    }

    u32 version() {
        return s_version;
    }

}
