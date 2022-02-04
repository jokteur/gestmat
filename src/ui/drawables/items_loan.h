#pragma once

#include "ui/drawable.h"
#include "ui/widgets/date.h"
#include "ui/drawables/select_item_widget.h"

int Search(ImGuiInputTextCallbackData* data);

class ItemsLoans : public Drawable {
private:
    Item::Manager_ptr m_manager = nullptr;
    std::string m_surname;
    std::string m_name;
    std::string m_place;

    std::vector<select_item_ptr> m_selected_items;
    std::set<Item::ItemID> m_selected_item_ids;

    std::map<std::string, Item::Person_ptr> m_search_results;
    Item::Person_ptr m_select_person = nullptr;

    bool is_selection_active = false;

    std::string m_save_error;

    enum Focus { SURNAME, NAME, NONE };
    Focus m_search_in = NONE;

    DateWidget m_birthday;
    DateWidget m_loan_date;
    Date m_current_date;

    std::string m_note;

    void reset();
    void save();

    void search_widget();
    void person_widget();

    friend int Search(ImGuiInputTextCallbackData* data);
public:
    ItemsLoans(UIState_ptr ui_state);

    void FrameUpdate() override;
    void BeforeFrameUpdate() override;
};