#pragma once

#include "ui/drawable.h"

class ItemsState : public Drawable {
private:
public:
    ItemsState(UIState_ptr ui_state) : Drawable(ui_state) {}

    void FrameUpdate() override;
    void BeforeFrameUpdate() override {}
};