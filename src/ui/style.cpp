#include <tempo.h>

void setLightStyle() {
    ImGui::StyleColorsLight();
    ImGuiStyle& style = ImGui::GetStyle();
    // Popup Bg
    style.Colors[ImGuiCol_PopupBg] = ImColor(240, 240, 240, 255);
    // style.Colors[ImGuiCol_Button] = ImColor(44, 90, 145, 102);

    style.FramePadding = ImVec2(10, 5);
    style.SelectableTextAlign = ImVec2(0, 0.5);
    style.FrameBorderSize = 1.0f;
    style.FrameRounding = 6.0f;
    style.ItemSpacing = ImVec2(8, 8);
}