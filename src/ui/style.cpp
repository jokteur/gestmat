#include <tempo.h>

void setLightStyle() {
    ImGui::StyleColorsLight();
    ImGuiStyle& style = ImGui::GetStyle();
    // Popup Bg
    style.Colors[ImGuiCol_PopupBg] = ImColor(240, 240, 240, 255);
}