#pragma once

#include "ui/drawable.h"
#include <memory>

#include "category_widget.h"

class ItemsManagement : public Drawable {
private:
    bool m_new_cat = false;

    std::string m_retire;
    std::string m_retire_error;

    std::map<Item::CategoryID, std::shared_ptr<CategoryWidget>> m_cat_widgets;

    std::shared_ptr<CategoryWidget> m_new_cat_widget;

public:
    ItemsManagement(UIState_ptr ui_state) : Drawable(ui_state) {}

    void FrameUpdate() override;
    void BeforeFrameUpdate() override;
};