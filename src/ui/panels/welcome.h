#pragma once

#include <map>
#include <string>
#include <vector>

#include "app/jobscheduler.h"
#include "app/settings.h"
#include "app/util.h"
#include "imgui.h"
#include "nfd.h"
#include "rendering/drawables.h"

namespace Rendering {
/**
     * Defines the UI for opening and exploring new folders
     */
class WelcomeView : public AbstractLayout {
   private:
   public:
    WelcomeView() = default;
    ~WelcomeView() override = default;

    /**
         * Draws ImGui elements to the window (between ImGui::NewFrame() and ImGui::Render())
         * @param window GLFW window pointer to which the drawable should be drawn
         * @param parent_dimension dimension of the parent layout
         */
    void ImGuiDraw(GLFWwindow* window, Rect& parent_dimension) override;
};
}  // namespace Rendering