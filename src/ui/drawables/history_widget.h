#pragma once

#include "ui/drawable.h"


class HistoryWidget : public Drawable {
private:
    Item::Manager_ptr m_manager = nullptr;

    ImVec2 m_vMin, m_vMax;
    ImVec2 m_window_pos, m_window_size;
    std::shared_ptr<Item::File> m_current_file = nullptr;

    bool m_show = false;
    bool m_set_scroll = true;
    bool m_visualize = false;

    void abandon();
public:
    HistoryWidget(UIState_ptr ui_state);

    void previewBar();
    void show() { m_show = true; m_set_scroll = true; }
    void FrameUpdate() override;
    void showPreview();
    void BeforeFrameUpdate() override;
};