#include "main_menu_bar.h"

#include "app/log.h"
#include "gui.h"
#include "ui/modales/modals.h"
#include "ui/shortcuts_list.h"

void Rendering::MainMenuBar::ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Fichier")) {
            ImGui::EndMenu();
        }
        if (!page_title_.empty()) {
            ImGui::SameLine((ImGui::GetWindowWidth() - 200.f) / 2);
            ImGui::TextDisabled(page_title_.c_str());
        }
        ImGui::EndMainMenuBar();
    }
}

Rendering::MainMenuBar::MainMenuBar()
    : event_queue_(EventQueue::getInstance()),
      settings_(Settings::getInstance()) {
    error_fct = [this](bool &show, bool &escape, bool &enter) {
        ImGui::Text("%s", error_msg.c_str());
        if (ImGui::Button("Ok") || escape || enter)
            show = false;
    };
    init_listeners();
}

void Rendering::MainMenuBar::init_listeners() {
}

void Rendering::MainMenuBar::destroy_listeners() {
}
