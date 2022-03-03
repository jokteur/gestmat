#pragma once

#include "ui/drawable.h"


class HistoryWidget : public Drawable {
private:
    Item::Manager_ptr m_manager = nullptr;

    ImVec2 m_vMin, m_vMax;
    ImVec2 m_window_pos, m_window_size;
    std::shared_ptr<Item::File> m_current_file = nullptr;
    std::vector<Item::File> m_files;

    bool m_show = false;
    bool m_is_set = false;
    bool m_set_scroll = true;
    bool m_visualize = false;

public:
    HistoryWidget(UIState_ptr ui_state);

    void abandon();
    void previewBar();
    void show(bool scroll = true);
    void FrameUpdate(std::vector<Item::File> files);
    void FrameUpdate() override;
    void showPreview();
    void BeforeFrameUpdate() override;
};