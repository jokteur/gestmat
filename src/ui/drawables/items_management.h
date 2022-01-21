#pragma once

#include "ui/drawable.h"

class ItemsManagement : public Drawable {
private:
public:
    ItemsManagement(UIState_ptr ui_state) : Drawable(ui_state) {}

    void FrameUpdate() override;
    void BeforeFrameUpdate() override {}
};