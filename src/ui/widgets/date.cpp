#include "date.h"

#include "../imgui_util.h"
#include "imgui_stdlib.h"

DateWidget::DateWidget(UIState_ptr ui_state) : Drawable(ui_state) {
    m_id = ui_state->imID;
    ui_state->imID++;
}

void DateWidget::reset() {
    m_id = m_ui_state->imID;
    m_ui_state->imID++;
    m_focus = DAY;
    m_day = "";
    m_month = "";
    m_year = "";
}

void DateWidget::setFocus() {
    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
        ImGui::SetKeyboardFocusHere(0);
}

void DateWidget::FrameUpdate() {
    if (m_focus == DAY) {
        if (m_day.size() == 2) {
            m_focus = MONTH;
        }
    }
    else if (m_focus == MONTH) {
        if (m_month.size() == MONTH) {
            m_focus = YEAR;
        }
    }
    else {

    }

    if (m_focus == DAY) {
        setFocus();
    }
    ImGui::SetNextItemWidth(40.f);
    ImGui::InputTextWithHint(labelize(m_id, "##jour").c_str(), "Jour", &m_day);
    ImGui::SameLine();

    if (m_focus == MONTH) {
        setFocus();
    }
    ImGui::SetNextItemWidth(40.f);
    ImGui::InputTextWithHint(labelize(m_id, "##mois").c_str(), "Mois", &m_month);
    ImGui::SameLine();

    if (m_focus == YEAR) {
        setFocus();
    }
    ImGui::SetNextItemWidth(80.f);
    ImGui::InputTextWithHint(labelize(m_id, "##annee").c_str(), "Année", &m_year);
    ImGui::SameLine();
}