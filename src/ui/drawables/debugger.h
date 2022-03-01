#pragma once

#include "ui/drawable.h"

namespace Debug {
    using namespace Item;
    class Debugger : public Drawable {
    private:
        Manager_ptr m_manager = nullptr;
        bool m_show = false;

        std::string m_global_id;

        std::map<LoanID, std::vector<ItemID>> m_duplicates;
        std::set<ItemID> m_discrepancies_item;
        std::set<PersonID> m_discrepancies_persons;
        std::set<LoanID> m_to_retire;
        std::set<LoanID> m_to_remove;
        void findBugs();

        void item_info(ItemID item_id);
        void person_info(PersonID person_id);

        void show_item(ItemID item_id, long long int counter);
        void show_person(PersonID person_id, long long int counter);
        void show_loan(LoanID loan_id, long long int counter);
    public:
        Debugger(UIState_ptr ui_state) : Drawable(ui_state) {}

        void show() { m_show = true; }

        void FrameUpdate() override;
        void BeforeFrameUpdate() override;
    };
}