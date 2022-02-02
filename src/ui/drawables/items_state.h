#pragma once

#include "ui/drawable.h"
#include "ui/widgets/filter.h"
#include "core/util.h"

class ItemsState : public Drawable {
private:
    Item::Manager_ptr m_manager = nullptr;

    // Sorting
    enum Cols { SURNAME, NAME, BIRTHDAY, PLACE, NOTES, LOAN_DATE, TYPE, ACTIONS };
    Cols m_sort_id = LOAN_DATE;
    bool m_ascending = true;
    bool m_fill_cols = true;

    FilterWidget m_filter;

    long long int m_sub_id;

    CheckBoxMap<Item::LoanID> m_loans_checkbox;

    std::map<std::string,
        std::map<std::string,
        std::vector<std::pair<
        Filter, Item::Loan_ptr>>>> m_loans;

    void give_back();

    void fill_items();
    void show_row(std::vector<std::pair<Filter, Item::Loan_ptr>> loan);
public:
    ItemsState(UIState_ptr ui_state);

    void FrameUpdate() override;
    void BeforeFrameUpdate() override;
};