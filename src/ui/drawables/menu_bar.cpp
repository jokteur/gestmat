#include "menu_bar.h"

void MenuBar::BeforeFrameUpdate() {
    auto m_manager = m_workspace.getCurrentManager();
    if (m_manager != nullptr) {
        auto durations = core::getDurations();
        auto duration = m_manager->getDuration();
        m_selected_duration[duration] = true;
        bool set_to_false = false;
        for (auto pair : durations) {
            if (m_selected_duration[pair.first]) {
                if (duration != pair.first) {
                    set_to_false = true;
                    m_manager->setDuration(pair.first);
                    continue;
                }
            }
        }
        if (set_to_false)
            m_selected_duration[duration] = false;
    }
}

void MenuBar::FrameUpdate() {
    ImGui::BeginMainMenuBar();

    if (ImGui::BeginMenu("Fichiers")) {
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Durée alertes")) {
        auto m_manager = m_workspace.getCurrentManager();
        if (m_manager != nullptr) {
            auto durations = core::getDurations();
            for (auto pair : durations) {
                ImGui::MenuItem(pair.second.c_str(), NULL, &m_selected_duration[pair.first]);
            }
        }
        ImGui::EndMenu();
    }
    float width = ImGui::GetContentRegionAvail().x;

    float button_width = ImGui::CalcTextSize("Historique").x;

    float margin = width - button_width - ImGui::GetStyle().ItemInnerSpacing.x * 4.5f;

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + margin);
    if (ImGui::Button("Historique")) {
        m_history.show();
    }

    ImGui::EndMainMenuBar();
    m_history.showPreview();
}