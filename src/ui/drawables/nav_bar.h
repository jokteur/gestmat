#pragma once

#include "ui/drawable.h"

class NavBar : public Drawable {
private:
public:
    NavBar(UIState_ptr ui_state) : Drawable(ui_state) {}

    void FrameUpdate() override;
    void BeforeFrameUpdate() override {}
};