#pragma once

#include <tempo.h>
#include <vector>
#include <string>


#include "ui/drawable.h"

class Combo : public Drawable {
private:
    std::vector<std::string> m_select;
    bool m_empty_first;
    int m_selected = -1;
    long long int m_id;

    void set_select(const std::vector<std::string>& select, bool empty_first);
public:
    Combo(UIState_ptr ui_state, std::vector<std::string> select, int default_select = 0, bool empty_first = false);
    Combo(UIState_ptr ui_state, std::vector<std::string> select, std::string default_select = "", bool empty_first = false);

    std::string getValue();
    int getSelected();

    void FrameUpdate() override;
    void BeforeFrameUpdate() override {}

    ~Combo() { }
};