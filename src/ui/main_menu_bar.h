#pragma once

#include <utility>

#include "app/events.h"
#include "app/jobscheduler.h"
#include "app/settings.h"
#include "first_include.h"
#include "imgui.h"
#include "nfd.h"
#include "rendering/drawables.h"
#include "ui/modales/error_message.h"

namespace Rendering {
class MainMenuBar : public AbstractLayout {
   private:
    EventQueue& event_queue_;
    Settings& settings_;
    Listener shortcuts_listener_;
    Listener page_title_change_;

    std::string error_msg;
    std::string page_title_ = "Gestion matériel";

    modal_fct error_fct;

    int num_projects = 0;
    bool close_projects_ = false;
    bool show_modal_ = false;

    void init_listeners();
    void destroy_listeners();

   public:
    MainMenuBar();

    void ImGuiDraw(GLFWwindow* window, Rect& parent_dimension) override;

    ~MainMenuBar() override { destroy_listeners(); }
};
}  // namespace Rendering
