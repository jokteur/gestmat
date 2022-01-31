#pragma once

#include "ui/drawable.h"

class Property : public Drawable {
private:
    Item::Manager_ptr m_manager;
    Item::PropertyID m_prop_id;
    Item::Property_ptr m_prop = nullptr;
    std::string m_name;
    bool m_mandatory;

    int m_choice = 0;
    std::vector<std::string> m_select;

    bool m_destroy_me = false;
    bool m_edit = false;
    bool m_same_name = false;

    void save();
public:
    Property(UIState_ptr ui_state, Item::PropertyID prop_id = -1);

    void FrameUpdate() override;
    void BeforeFrameUpdate() override;

    bool destroy_me() { return m_destroy_me; }
};

class PropertiesWidget : public Drawable {
private:
    Item::Manager_ptr m_manager;

    std::string m_delete;
    std::string m_delete_error;

    std::map<Item::PropertyID, std::shared_ptr<Property>> m_edit_widgets;

    void delete_warning(Item::Property_ptr prop);
public:
    PropertiesWidget(UIState_ptr ui_state);

    void FrameUpdate() override;
    void BeforeFrameUpdate() override;
};