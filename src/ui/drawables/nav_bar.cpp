#include "nav_bar.h"

void NavBar::FrameUpdate() {
    // Styling the tab bar
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20, 8));

    ImGui::BeginTabBar("Nav bar");
    if (ImGui::BeginTabItem("Prêt matériel")) {
        m_ui_state->active_panel = UIState::LOAN;
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("État des emprunts")) {
        m_ui_state->active_panel = UIState::STATE;
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Gestion matériel")) {
        if (m_ui_state->active_panel != UIState::PROPERTIES)
            m_ui_state->active_panel = UIState::MANAGEMENT;
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Statistiques")) {
        m_ui_state->active_panel = UIState::ALERTES;
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();

    ImGui::PopStyleVar();
}