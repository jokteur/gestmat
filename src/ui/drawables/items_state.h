#pragma once

#include "ui/drawable.h"

class ItemsState : public Drawable {
private:
    Item::Manager_ptr m_manager = nullptr;

    // Sorting
    enum Cols { SURNAME, NAME, BIRTHDAY, PLACE, NOTES, LOAN_DATE, TYPE };
    Cols m_sort_id = LOAN_DATE;
    bool m_ascending = true;
    bool m_fill_cols = true;

    long long int m_sub_id;

    std::map<std::string, std::map<std::string, std::vector<Item::Loan_ptr>>> m_loans;

    void fill_items();
    void show_row(std::vector<Item::Loan_ptr> loan);
public:
    ItemsState(UIState_ptr ui_state);

    void FrameUpdate() override;
    void BeforeFrameUpdate() override;
};