#include "combo.h"

#include "imgui_stdlib.h"
#include "ui/imgui_util.h"

Combo::Combo(UIState_ptr ui_state, std::vector<std::string> select, int default_select, bool empty_first)
    : Drawable(ui_state) {
    m_selected = default_select;
    if (empty_first) {
        m_select.push_back(" ");
    }
    for (auto& str : select) {
        m_select.push_back(str);
    }
    if (default_select < 0 || default_select >= m_select.size()) {
        m_selected = 0;
    }
    m_id = m_ui_state->imID;
    m_ui_state->imID++;
}
Combo::Combo(UIState_ptr ui_state, std::vector<std::string> select, std::string default_select, bool empty_first)
    : Drawable(ui_state) {

    int i = 0;
    if (empty_first) {
        m_select.push_back(" ");
        i++;
    }
    for (auto& str : select) {
        if (str == default_select) {
            m_selected = i;
        }
        i++;
        m_select.push_back(str);
    }
    if (m_selected == -1) {
        m_selected = 0;
    }
    m_id = m_ui_state->imID;
    m_ui_state->imID++;
}

std::string Combo::getValue() {
    std::string str = m_select[m_selected];
    if (str == "##empty_select")
        str = "";
    return str;
}

int Combo::getSelected() {
    return m_selected;
}

void Combo::FrameUpdate() {
    if (ImGui::BeginCombo(labelize(m_id, "combo_select").c_str(), m_select[m_selected].c_str())) {
        for (int n = 0;n < m_select.size();n++) {
            bool is_selected = m_select[n] == m_select[m_selected];
            if (ImGui::Selectable(m_select[n].c_str(), is_selected)) {
                m_selected = n;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
}