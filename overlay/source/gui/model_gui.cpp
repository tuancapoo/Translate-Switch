#include "gui/model_gui.hpp"

#include "app/provider_choice.hpp"
#include "config.hpp"

namespace {

    bool isPreset(const std::string& providerId, const std::string& model) {
        for (const auto& preset : provider_choice::presets(providerId)) {
            if (model == preset.id)
                return true;
        }
        return false;
    }

}

tsl::elm::Element* ModelGui::createUI() {
    auto frame = new tsl::elm::OverlayFrame(config::AppName, "Chọn model");
    auto list = new tsl::elm::List();

    const std::string provider = provider_choice::currentProvider();
    const std::string current = provider_choice::currentModel();   // "" = mặc định
    const std::string effective = current.empty() ? provider_choice::defaultModel(provider) : current;

    list->addItem(new tsl::elm::CategoryHeader(provider_choice::label(provider)));

    auto addChoice = [&](const std::string& modelId, const std::string& title, const std::string& value) {
        auto item = new tsl::elm::ListItem(title, value);
        item->setClickListener([this, modelId](u64 keys) {
            if (!(keys & HidNpadButton_A))
                return false;
            provider_choice::setModel(modelId);
            m_done = true;
            return true;
        });
        list->addItem(item);
    };

    const auto& presets = provider_choice::presets(provider);
    if (presets.empty()) {
        // OpenRouter / API chuẩn OpenAI: backend không có model mặc định
        list->addItem(new tsl::elm::CategoryHeader("Provider này cần đặt model trong config.ini"));
        list->addItem(new tsl::elm::ListItem(current.empty() ? "model=(chưa đặt)" : "model=" + current));
    } else {
        // Model đang đặt tay trong config.ini mà không có trong danh sách gợi ý
        if (!current.empty() && !isPreset(provider, current))
            addChoice(current, current, "Đang dùng (config.ini)");

        for (const auto& preset : presets) {
            const bool selected = preset.id == effective;
            std::string value = selected ? "Đang dùng" : preset.note;
            addChoice(preset.id == provider_choice::defaultModel(provider) ? "" : preset.id, preset.id, value);
        }
    }

    list->addItem(new tsl::elm::CategoryHeader("Model khác: sửa model=... trong config.ini"));

    frame->setContent(list);
    return frame;
}

void ModelGui::update() {
    if (m_done)
        tsl::goBack();
}
