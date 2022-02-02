#pragma once

#include "ui/drawable.h"
#include "ui/widgets/date.h"

class ItemsLoans : public Drawable {
private:
    Item::Manager_ptr m_manager = nullptr;
    std::string m_surname;
    std::string m_name;
    std::string m_place;

    DateWidget m_birthday;

    std::string m_note;

    void person_widget();
public:
    ItemsLoans(UIState_ptr ui_state);

    void FrameUpdate() override;
    void BeforeFrameUpdate() override {}
};