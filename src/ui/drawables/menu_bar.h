#pragma once

#include "ui/drawable.h"
#include "history_widget.h"
#include "ui/widgets/combo.h"
#include "ui/drawables/debugger.h"
#include "ui/drawables/restore.h"

class MenuBar : public Drawable {
private:
    HistoryWidget m_history;
    bool demo = false;

    bool show_window = false;

    CheckBoxMap<Duration> m_selected_duration;
    Restore m_restore_window;
    Debug::Debugger m_debugger;
public:
    MenuBar(UIState_ptr ui_state) : Drawable(ui_state), m_history(ui_state), m_debugger(ui_state), m_restore_window(ui_state) {}

    void FrameUpdate() override;
    void BeforeFrameUpdate() override;
    void DrawHistory() { m_history.FrameUpdate(); }
};