#pragma once

#include "ui/drawable.h"

class ItemsLoans : public Drawable {
private:
public:
    ItemsLoans(UIState_ptr ui_state) : Drawable(ui_state) {}

    void FrameUpdate() override;
    void BeforeFrameUpdate() override {}
};