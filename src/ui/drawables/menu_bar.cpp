#include "menu_bar.h"

void MenuBar::FrameUpdate() {
    ImGui::BeginMainMenuBar();

    if (ImGui::BeginMenu("Fichiers")) {
        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}