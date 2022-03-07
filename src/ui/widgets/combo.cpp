#include "combo.h"

#include "imgui_stdlib.h"
#include "ui/imgui_util.h"

Combo::Combo(UIState_ptr ui_state, std::vector<std::string> select, int default_select, bool empty_first, bool max_size)
    : Drawable(ui_state) {
    m_selected = default_select;
    m_max_length = max_size;
    if (empty_first) {
        m_select.push_back("##empty_select");
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
Combo::Combo(UIState_ptr ui_state, std::vector<std::string> select, std::string default_select, bool empty_first, bool max_size)
    : Drawable(ui_state) {

    int i = 0;
    m_max_length = max_size;
    if (empty_first) {
        m_select.push_back("##empty_select");
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

void Combo::set_select(const std::vector<std::string>& select, bool empty_first) {
    int i = 0;
    m_select.clear();
    if (empty_first) {
        m_select.push_back("##empty_select");
        i++;
    }
    for (auto& str : select) {
        i++;
        m_select.push_back(str);
    }
    m_selected = 0;

    m_id = m_ui_state->imID;
    m_ui_state->imID++;
}

void Combo::setSelect(const std::vector<std::string>& select, bool empty_first) {
    set_select(select, empty_first);
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
    ImGuiComboFlags flags = 0;
    if (m_max_length)
        flags |= ImGuiComboFlags_HeightLargest;
    if (ImGui::BeginCombo(labelize(m_id, "##combo_select").c_str(), m_select[m_selected].c_str(), flags)) {
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