#include "misc.h"

void title(std::string title, UIState_ptr ui_state) {
    Tempo::PushFont(ui_state->font_title);
    ImGui::Text(title.c_str());
    Tempo::PopFont();
}

bool button(std::string name, UIState_ptr ui_state, std::string deactivated_msg, ImVec4 color, ImVec2 size) {
    bool is_deactivated = ui_state->read_only || !deactivated_msg.empty();
    bool colored = color.w != 0.f && color.x != 0.f && color.y != 0.f && color.z != 0.f;
    if (colored) {
        ImGui::PushStyleColor(ImGuiCol_Button, color);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(color.x * 0.9f, color.y * 0.9f, color.z * 0.9f, color.w + 0.1f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(color.x * 0.8f, color.y * 0.8f, color.z * 0.8f, color.w + 0.2f));
    }
    if (is_deactivated) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    }
    bool ret = ImGui::Button(name.c_str(), size);
    if (is_deactivated) {
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
            ImGui::BeginTooltip();
            if (!deactivated_msg.empty())
                ImGui::Text(deactivated_msg.c_str());
            else
                ImGui::Text("Boutton désactivé temporairement");
            ImGui::EndTooltip();
        }
        ImGui::PopItemFlag();
        ImGui::PopStyleVar();
    }
    if (colored) {
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
    }
    return ret & !is_deactivated;
}
void labeledTextInput(std::string* content, const std::string& label, const std::string& imId, const std::string& hint, const std::string& error, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data) {
    ImGui::AlignTextToFramePadding();
    ImGui::Text(label.c_str());
    ImGui::SameLine();
    if (!error.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(125, 0, 0, 79));
    }
    ImGui::InputTextWithHint(imId.c_str(), hint.c_str(), content, flags, callback, user_data);
    if (!error.empty()) {
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(125, 0, 0, 79));
        ImGui::Text(error.c_str());
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
    }
}

void help(std::string content) {
    ImGui::TextColored(ImVec4(0.2f, 0.2f, 0.2f, 0.4f), "(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text(content.c_str());
        ImGui::EndTooltip();
    }
}