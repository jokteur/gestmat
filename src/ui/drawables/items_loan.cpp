#include "items_loan.h"

#include "ui/widgets/misc.h"
#include "ui/imgui_util.h"
#include "imgui_stdlib.h"

ItemsLoans::ItemsLoans(UIState_ptr ui_state) : Drawable(ui_state), m_birthday(ui_state) {

}

void ItemsLoans::person_widget() {
    Tempo::PushFont(m_ui_state->font_bold);
    ImGui::Text("Informations sur patient");
    Tempo::PopFont();

    float pos_x = ImGui::GetCursorPosX();
    const float spacing = 200.f;

    ImGui::Text("Nom:"); ImGui::SameLine();
    ImGui::SetCursorPosX(pos_x + spacing);
    ImGui::InputText("##in_surname", &m_surname);

    ImGui::Text("Prénom:"); ImGui::SameLine();
    ImGui::SetCursorPosX(pos_x + spacing);
    ImGui::InputText("##in_name", &m_surname);

    ImGui::Text("Date de naissance:"); ImGui::SameLine();
    ImGui::SetCursorPosX(pos_x + spacing);
    m_birthday.FrameUpdate();
}

void ItemsLoans::FrameUpdate() {
    m_manager = m_workspace.getCurrentManager();
}