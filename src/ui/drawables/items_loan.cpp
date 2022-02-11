#include "items_loan.h"

#include "ui/widgets/misc.h"
#include "ui/widgets/modal.h"
#include "ui/imgui_util.h"
#include "imgui_stdlib.h"
#include "imgui_internal.h"

ItemsLoans::ItemsLoans(UIState_ptr ui_state) : Drawable(ui_state), m_birthday(ui_state), m_loan_date(ui_state) {
    reset();
}
void ItemsLoans::reset() {
    m_surname = "";
    m_name = "";
    m_place = "";
    m_selected_items.clear();
    m_selected_item_ids.clear();
    m_select_person = nullptr;
    m_current_date = getCurrentDate();
    m_selected_items.push_back(std::make_shared<SelectItemWidget>(m_ui_state));
    m_search_results.clear();
    m_birthday.reset();
    m_loan_date.setToday();
    m_note = "";
    m_search_in = NONE;
}

void ItemsLoans::save() {
    if (m_selected_items.empty())
        return;

    Item::LoanID prev_loan = -1;

    for (auto item_id : m_selected_item_ids) {
        auto loan_res = m_manager->findLoans(item_id);
        if (loan_res.has_value()) {
            for (auto loan_id : loan_res.value())
                m_manager->retireLoan(loan_id, getCurrentDate());
        }
        Item::Note note{ .content = m_note,.timestamp = getTimestamp() };
        if (prev_loan != -1)
            note = {};

        if (m_select_person != nullptr) {
            prev_loan = m_manager->newLoan(
                item_id,
                note,
                m_current_date,
                m_select_person->id).value();
        }
        else if (prev_loan != -1) {
            auto loan = m_manager->getLoan(prev_loan).value();
            prev_loan = m_manager->newLoan(
                item_id,
                note,
                m_current_date,
                loan->person).value();
        }
        else {
            prev_loan = m_manager->newLoan(
                item_id,
                note,
                m_current_date,
                Item::Person{
                    .name = m_name,
                    .surname = m_surname,
                    .place = m_place,
                    .notes = {},
                    .birthday = m_birthday.getDate()
                }).value();
        }
    }
    m_workspace.save("emprunt_objets");
    reset();
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
    float item_height = ImGui::GetTextLineHeightWithSpacing();
    const float spacing = item_height * 6;

    ImGui::Text("Nom:"); ImGui::SameLine();
    ImGui::SetCursorPosX(pos_x + spacing);
    if (m_select_person == nullptr) {
        ImGui::SetNextItemWidth(spacing);
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
        ImGui::SetCursorPosX(pos_x + spacing * 2);
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
        ImGui::SetNextItemWidth(spacing);
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
        ImGui::SetNextItemWidth(spacing);
        ImGui::InputText("##in_place", &m_place);
    }
    else {
        ImGui::SetCursorPosY(padding + ImGui::GetCursorPosY());
        ImGui::Text(m_select_person->place.c_str());
        ImGui::SetCursorPosY(padding + ImGui::GetCursorPosY());
    }

    ImGui::Text("Remarque:"); ImGui::SameLine();
    ImGui::SetCursorPosX(pos_x + spacing);
    ImGui::SetNextItemWidth(spacing);
    ImGui::InputTextMultiline("##in_note", &m_note, ImVec2(0, 100.f));
}

void ItemsLoans::BeforeFrameUpdate() {
    m_birthday.BeforeFrameUpdate();
    m_loan_date.BeforeFrameUpdate();

    std::set<std::vector<select_item_ptr>::iterator> to_destroy;
    for (auto it = m_selected_items.begin();it != m_selected_items.end();it++) {
        (*it)->BeforeFrameUpdate();
        if ((*it)->destroy_me()) {
            to_destroy.insert(it);
            auto item_id = (*it)->getSelectedItem();
            if (item_id != -1)
                m_selected_item_ids.erase(item_id);
        }
    }
    for (auto it : to_destroy) {
        m_selected_items.erase(it);
    }

}

void ItemsLoans::FrameUpdate() {
    m_manager = m_workspace.getCurrentManager();
    title("Prêt", m_ui_state);
    ImGui::SameLine();
    m_save_error = "Merci de choisir au moins un objet avant de sauvegarder";
    if (!m_selected_item_ids.empty())
        m_save_error = "";
    if (button("Sauvegarder##save_loan", m_ui_state, m_save_error)) {
        save();
    }
    ImGui::Separator();

    ImGui::BeginChild("Pret_window");
    person_widget();

    ImGui::AlignTextToFramePadding();
    Tempo::PushFont(m_ui_state->font_bold);
    ImGui::Text("Date d'emprunt:");
    Tempo::PopFont();
    ImGui::SameLine();

    ImGui::Text(m_current_date.format("%d/%m/%Y").c_str());
    ImGui::SameLine();
    button("Changer##change_date", m_ui_state);
    if (ImGui::BeginPopupContextItem(0, ImGuiPopupFlags_MouseButtonLeft)) {
        m_loan_date.FrameUpdate();
        ImGui::SameLine();
        if (button("OK##close_popup", m_ui_state)) {
            auto date = m_loan_date.getDate();
            if (date.isValid()) {
                m_current_date = date;
            }
            else {
                m_loan_date.setToday();
                m_current_date = getCurrentDate();
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    Tempo::PushFont(m_ui_state->font_bold);
    ImGui::Text("Objet(s) à emprunter");
    Tempo::PopFont();

    std::string error;
    if (is_selection_active) {
        error = "Merci de finir de choisir un premier objet avant d'ajouter un deuxième";
    }
    if (button("+##add_item", m_ui_state, error)) {
        m_selected_items.push_back(std::make_shared<SelectItemWidget>(m_ui_state));
    }

    is_selection_active = false;
    for (auto& widget : m_selected_items) {
        widget->FrameUpdate();
        auto item_id = widget->getSelectedItem();
        widget->avoidTheseItems(m_selected_item_ids);
        if (item_id != -1) {
            m_selected_item_ids.insert(item_id);
        }
        else {
            is_selection_active = true;
        }
    }
    ImGui::EndChild();
}