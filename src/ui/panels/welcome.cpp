#include "welcome.h"
namespace Rendering {
void WelcomeView::ImGuiDraw(GLFWwindow* window, Rect& parent_dimension) {
    ImGui::Begin("Welcome page");
    ImVec2 content = ImGui::GetContentRegionAvail();

    ImGui::Text("What do you want to do ?");
    ImGui::End();
}
}  // namespace Rendering