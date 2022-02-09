#include "menu_bar.h"

void MenuBar::FrameUpdate() {
    ImGui::BeginMainMenuBar();

    if (ImGui::BeginMenu("Fichiers")) {
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