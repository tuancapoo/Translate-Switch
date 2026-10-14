#include "gui/provider_gui.hpp"

#include "app/provider_choice.hpp"
#include "config.hpp"

tsl::elm::Element* ProviderGui::createUI() {
    auto frame = new tsl::elm::OverlayFrame(config::AppName, "Chọn provider");
    auto list = new tsl::elm::List();

    list->addItem(new tsl::elm::CategoryHeader("Đổi provider sẽ đặt lại model"));

    const std::string current = provider_choice::currentProvider();
    for (const auto& provider : provider_choice::providers()) {
        auto item = new tsl::elm::ListItem(provider.label, current == provider.id ? "Đang dùng" : "");
        item->setClickListener([this, id = std::string(provider.id)](u64 keys) {
            if (!(keys & HidNpadButton_A))
                return false;
            provider_choice::setProvider(id);
            m_done = true;
            return true;
        });
        list->addItem(item);
    }

    list->addItem(new tsl::elm::CategoryHeader("API key đặt trong config.ini (api_key=...)"));

    frame->setContent(list);
    return frame;
}

void ProviderGui::update() {
    if (m_done)
        tsl::goBack();
}
