#pragma once

#include "ui/drawable.h"
#include "history_widget.h"

class Restore : public Drawable {
private:
    std::shared_ptr<HistoryWidget> m_history;
    std::vector<Item::File> m_files;

    bool m_show_window = false;
    bool m_just_closed = false;
    bool m_restoring = false;
    std::string error_msg = "";
    std::string confirmation;

public:
    Restore(UIState_ptr ui_state) : Drawable(ui_state) {}

    void FrameUpdate() override;
    // void BeforeFrameUpdate();

    void show();
};