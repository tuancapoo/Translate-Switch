#pragma once

#include <tesla.hpp>

// Màn hình chính của overlay

class MainGui : public tsl::Gui {
public:
    tsl::elm::Element* createUI() override;
    void update() override;

private:
    tsl::elm::ListItem* m_statusItem = nullptr;
    tsl::elm::ListItem* m_resultsItem = nullptr;
    tsl::elm::ListItem* m_hotkeyItem = nullptr;
    tsl::elm::ListItem* m_opacityItem = nullptr;
    tsl::elm::ListItem* m_providerItem = nullptr;
    tsl::elm::ListItem* m_modelItem = nullptr;
    u32 m_shownChoiceVersion = 0;
    u64 m_shownHotkey = 0;
    u32 m_shownOpacityVersion = 0;
    u32 m_shownStatusVersion = 0;
    u32 m_shownTranslationVersion = 0;
};
