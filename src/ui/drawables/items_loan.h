#pragma once

#include "ui/drawable.h"
#include "ui/widgets/date.h"

int Search(ImGuiInputTextCallbackData* data);

class ItemsLoans : public Drawable {
private:
    Item::Manager_ptr m_manager = nullptr;
    std::string m_surname;
    std::string m_name;
    std::string m_place;

    std::map<std::string, Item::Person_ptr> m_search_results;
    Item::Person_ptr m_select_person = nullptr;

    enum Focus { SURNAME, NAME, NONE };
    Focus m_search_in = NONE;

    DateWidget m_birthday;

    std::string m_note;

    void search_widget();
    void person_widget();

    friend int Search(ImGuiInputTextCallbackData* data);
public:
    ItemsLoans(UIState_ptr ui_state);

    void FrameUpdate() override;
    void BeforeFrameUpdate() override;
};