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
    u32 m_shownStatusVersion = 0;
    u32 m_shownTranslationVersion = 0;
};
