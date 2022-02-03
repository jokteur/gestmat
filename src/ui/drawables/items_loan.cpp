#include "items_loan.h"

#include "ui/widgets/misc.h"
#include "ui/imgui_util.h"
#include "imgui_stdlib.h"
#include "imgui_internal.h"

ItemsLoans::ItemsLoans(UIState_ptr ui_state) : Drawable(ui_state), m_birthday(ui_state) {

}

int Search(ImGuiInputTextCallbackData* data) {
    auto widget = static_cast<ItemsLoans*>(data->UserData);
    if (widget == nullptr) {
        return 1;
    }

    if (data->BufTextLen == 0) {
        widget->m_search_results.clear();
        return 0;
    }
    if (widget->m_search_in == ItemsLoans::NONE)
        return 1;

    widget->m_search_results.clear();
    auto loans = widget->m_manager->getActiveLoans();
    std::string input = toLower(std::string(data->Buf));

    for (auto loan_id : loans) {
        auto loan = widget->m_manager->getLoan(loan_id).value();
        auto person = widget->m_manager->getPerson(loan->person).value();

        std::string name;
        if (widget->m_search_in == ItemsLoans::SURNAME) {
            name = toLower(person->surname);
        }
        if (widget->m_search_in == ItemsLoans::NAME) {
            name = toLower(person->name);
        }
        if (name.find(input) == 0) {
            widget->m_search_results[person->surname] = person;
        }
    }
    return 0;
}

void ItemsLoans::search_widget() {
    ImGui::SameLine();
    float width = ImGui::GetContentRegionAvailWidth() - 50.f;
    float height = 250.f;
    if (m_search_in == SURNAME) {
        ImGui::Text("<");
    }
    else {
        float pos_y = ImGui::GetCursorPosY();
        float line_height = 1.5f * ImGui::GetTextLineHeight()
            + ImGui::GetStyle().ItemSpacing.y / 2;
        ImGui::SetCursorPosY(line_height + pos_y);
        ImGui::Text("<");
        ImGui::SetCursorPosY(pos_y - line_height);
        height -= line_height;
    }
    ImGui::SameLine();
    ImGui::BeginChild("Proposition de personnes", ImVec2(width, height), true);
    Tempo::PushFont(m_ui_state->font_bold);
    ImGui::Text("Proposition de personnes");
    Tempo::PopFont();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 2));
    for (auto pair : m_search_results) {
        auto person = pair.second;
        if (button(labelize(person->id, "Choisir"), m_ui_state)) {
            m_select_person = person;
        }
        ImGui::SameLine();
        ImGui::Text(person->surname.c_str());
        ImGui::SameLine();
        ImGui::Text(person->name.c_str());
        ImGui::SameLine();
        ImGui::Text(person->birthday.format("%d/%m/%Y").c_str());
    }
    ImGui::PopStyleVar();
    ImGui::EndChild();
}

void ItemsLoans::person_widget() {
    Tempo::PushFont(m_ui_state->font_bold);
    ImGui::Text("Informations sur patient");
    Tempo::PopFont();

    float pos_x = ImGui::GetCursorPosX();
    const float spacing = 200.f;

    ImGui::Text("Nom:"); ImGui::SameLine();
    ImGui::SetCursorPosX(pos_x + spacing);
    if (m_select_person == nullptr) {
        ImGui::SetNextItemWidth(200.f);
        ImGui::InputText("##in_surname", &m_surname, ImGuiInputTextFlags_CallbackEdit, Search, this);
        if (ImGui::IsItemActive()) {
            m_search_in = SURNAME;
        }
    }
    else {
        ImGui::AlignTextToFramePadding();
        ImGui::Text(m_select_person->surname.c_str());

        // I want that the button is here
        m_search_results.clear();
        ImGui::SameLine();
        ImGui::SetCursorPosX(pos_x + 400.f);
        if (button("Enlever personne", m_ui_state)) {
            m_select_person = nullptr;
        }
    }

    float pos_y = ImGui::GetCursorPosY();
    float padding = (ImGui::GetStyle().FramePadding.y) / 2.f;
    if (!m_search_results.empty()) {
        search_widget();
        ImGui::SetCursorPos(ImVec2(pos_x, pos_y));
    }

    ImGui::Text("Prénom:"); ImGui::SameLine();
    ImGui::SetCursorPosX(pos_x + spacing);
    if (m_select_person == nullptr) {
        ImGui::SetNextItemWidth(200.f);
        ImGui::InputText("##in_name", &m_name, ImGuiInputTextFlags_CallbackEdit, Search, this);
        if (ImGui::IsItemActive()) {
            m_search_in = NAME;
        }
    }
    else {
        ImGui::SetCursorPosY(padding + ImGui::GetCursorPosY());
        ImGui::Text(m_select_person->name.c_str());
        ImGui::SetCursorPosY(padding + ImGui::GetCursorPosY());
    }

    ImGui::Text("Date de naissance:"); ImGui::SameLine();
    ImGui::SetCursorPosX(pos_x + spacing);
    if (m_select_person == nullptr)
        m_birthday.FrameUpdate();
    else {
        ImGui::SetCursorPosY(padding + ImGui::GetCursorPosY());
        ImGui::Text(m_select_person->birthday.format("%d/%m/%Y").c_str());
        ImGui::SetCursorPosY(padding + ImGui::GetCursorPosY());
    }

    ImGui::Text("Unité/chambre:"); ImGui::SameLine();
    ImGui::SetCursorPosX(pos_x + spacing);
    if (m_select_person == nullptr) {
        ImGui::SetNextItemWidth(200.f);
        ImGui::InputText("##in_place", &m_place);
    }
    else {
        ImGui::SetCursorPosY(padding + ImGui::GetCursorPosY());
        ImGui::Text(m_select_person->place.c_str());
        ImGui::SetCursorPosY(padding + ImGui::GetCursorPosY());
    }

    ImGui::Text("Remarque:"); ImGui::SameLine();
    ImGui::SetCursorPosX(pos_x + spacing);
    ImGui::SetNextItemWidth(200.f);
    ImGui::InputTextMultiline("##in_note", &m_note, ImVec2(0, 100.f));

}

void ItemsLoans::BeforeFrameUpdate() {
    m_birthday.BeforeFrameUpdate();
}

void ItemsLoans::FrameUpdate() {
    m_manager = m_workspace.getCurrentManager();
    person_widget();
}