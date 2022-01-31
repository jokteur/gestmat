#pragma once

#include "ui/drawable.h"

class ItemsAlert : public Drawable {
private:
public:
    ItemsAlert(UIState_ptr ui_state) : Drawable(ui_state) {}

    void FrameUpdate() override;
    void BeforeFrameUpdate() override {}
};