#pragma once

#include "ui/drawable.h"
#include <memory>
#include <map>

#include "core/util.h"

using namespace core;


class CategoryWidget : public Drawable {
private:
    std::string m_name;
    std::string m_prev_name;
    Item::CategoryID m_cat_id;
    bool m_same_name = false;
    bool m_empty_prop = false;

    Item::Manager_ptr m_manager;
    Item::Category_ptr m_category = nullptr;

    CheckBoxMap<Item::PropertyID> m_properties_hide;

    bool m_destroy = false;
    std::set<Item::PropertyID> m_current_properties;
    std::vector<Item::PropertyID> m_current_properties_order;

    void PropertiesWidget();
    void save();
public:
    CategoryWidget(UIState_ptr ui_state, Item::CategoryID cat_id = -1);
    void FrameUpdate() override;
    void BeforeFrameUpdate() override;

    bool destroy_me() { return m_destroy; }
};