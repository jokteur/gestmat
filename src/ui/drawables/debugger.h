#pragma once

#include "ui/drawable.h"

namespace Debug {
    using namespace Item;
    class Debugger : public Drawable {
    private:
        Manager_ptr m_manager = nullptr;
        bool m_show = false;

        std::map<LoanID, std::vector<ItemID>> m_duplicates;
        void findBugs();

        void item_info(ItemID item_id);
        void person_info(PersonID person_id);
    public:
        Debugger(UIState_ptr ui_state) : Drawable(ui_state) {}

        void show() { m_show = true; }

        void FrameUpdate() override;
        void BeforeFrameUpdate() override;
    };
}