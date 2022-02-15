#pragma once

#include "ui/drawable.h"
#include "history_widget.h"
#include "ui/widgets/combo.h"

class MenuBar : public Drawable {
private:
    HistoryWidget m_history;
    bool demo = false;

    CheckBoxMap<Duration> m_selected_duration;
public:
    MenuBar(UIState_ptr ui_state) : Drawable(ui_state), m_history(ui_state) {}

    void FrameUpdate() override;
    void BeforeFrameUpdate() override;
    void DrawHistory() { m_history.FrameUpdate(); }
};