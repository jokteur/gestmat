#pragma once

#include "ui/drawable.h"
#include "history_widget.h"

class MenuBar : public Drawable {
private:
    HistoryWidget m_history;
    bool demo = false;
public:
    MenuBar(UIState_ptr ui_state) : Drawable(ui_state), m_history(ui_state) {}

    void FrameUpdate() override;
    void BeforeFrameUpdate() override {}
    void DrawHistory() { m_history.FrameUpdate(); }
};