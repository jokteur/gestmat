#include "items_state.h"

#include "ui/widgets/misc.h"
#include "ui/imgui_util.h"
#include "ui/widgets/modal.h"

void ItemsState::BeforeFrameUpdate() {
}


ItemsState::ItemsState(UIState_ptr ui_state) : Drawable(ui_state) {
}

void ItemsState::fill_items() {
    m_loans.clear();
    for (auto loan_id : m_manager->getActiveLoans()) {
        auto loan = m_manager->getLoan(loan_id).value();
        auto item = m_manager->getItem(loan->item).value();
        auto cat = m_manager->getCategory(item->category).value();
        auto person = m_manager->getPerson(loan->person).value();

        std::string key = person->surname + person->name;
        if (m_sort_id == TYPE) {
            m_loans[cat->name][key].push_back(loan);
        }
        else if (m_sort_id == SURNAME) {
            m_loans[person->surname][key].push_back(loan);
        }
        else if (m_sort_id == NAME) {
            m_loans[person->name][key].push_back(loan);
        }
        else if (m_sort_id == BIRTHDAY) {
            m_loans[person->birthday.format("%Y/%m/%d")][key].push_back(loan);
        }
        else if (m_sort_id == PLACE) {
            m_loans[person->place][key].push_back(loan);
        }
        else {
            m_loans[loan->date.format("%Y/%m/%d")][key].push_back(loan);
        }
    }
}


void ItemsState::show_row(std::vector<Item::Loan_ptr> loans) {
    ImGui::TableNextRow();

    auto person = m_manager->getPerson(loans[0]->person).value();

    ImGui::TableSetColumnIndex(SURNAME);
    ImGui::Text(person->surname.c_str());

    ImGui::TableSetColumnIndex(NAME);
    ImGui::Text(person->name.c_str());

    ImGui::TableSetColumnIndex(BIRTHDAY);
    ImGui::Text(person->birthday.format("%Y/%m/%d").c_str());

    ImGui::TableSetColumnIndex(PLACE);
    ImGui::Text(person->place.c_str());

    ImGui::TableSetColumnIndex(NOTES);
    for (auto& note : person->notes) {
        Tempo::PushFont(m_ui_state->font_bold);
        timestampToText(note.timestamp);
        Tempo::PopFont();
        ImGui::Text(note.content.c_str());
    }

    // for (auto loan : loans) {
    //     Item::Item_ptr item = m_manager->getItem(loan->item).value();
    //     Item::Category_ptr cat = m_manager->getCategory(item->category).value();

        // ImGui::TableSetColumnIndex(ACTIONS);
        // button(labelize(loan->id, "Actions"), m_ui_state);
        // if (ImGui::BeginPopupContextItem(0, ImGuiPopupFlags_MouseButtonLeft)) {
        //     if (button(labelize(loan->id, "Rendre objet"), m_ui_state)) {

        //     }
        //     if (button(labelize(loan->id, "Éditer patient"), m_ui_state)) {

        //     }
        //     if (button(labelize(loan->id, "Rendre tous les objets"), m_ui_state)) {

        //     }
        //     ImGui::EndPopup();
        // }
    // }
    int i = 0;
    Tempo::PushFont(m_ui_state->font_bold);
    for (auto loan : loans) {
        ImGui::TableSetColumnIndex(LOAN_DATE);
        if (!i)
            ImGui::Text("");
        ImGui::Text(loan->date.format("%Y/%m/%d").c_str());
        i++;
    }
    Tempo::PopFont();
    i = 0;
    for (auto loan : loans) {
        Item::Item_ptr item = m_manager->getItem(loan->item).value();
        Item::Category_ptr cat = m_manager->getCategory(item->category).value();

        ImGui::TableSetColumnIndex(TYPE);
        if (!i)
            ImGui::Text("");
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 0.7f), "(infos) ");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            for (auto prop_id : cat->properties) {
                if (cat->properties_hide.contains(prop_id))
                    continue;
                auto prop = m_manager->getProperty(prop_id).value();
                Tempo::PushFont(m_ui_state->font_bold);
                ImGui::Text((prop->name + " ").c_str());
                Tempo::PopFont();
                ImGui::SameLine();
                ImGui::Text(item->property_values[prop_id].c_str());
            }
            ImGui::EndTooltip();
        }
        ImGui::SameLine();
        ImGui::Text(cat->name.c_str());
        i++;
    }
}

void ItemsState::FrameUpdate() {
    m_manager = m_workspace.getCurrentManager();
    if (m_sub_id == 0) {
        m_sub_id = m_manager->getId();
    }
    title("Emprunts en cours", m_ui_state);

    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5, 5));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 2));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);

    ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable
        | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable
        | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY
        | ImGuiTableFlags_ScrollX;

    if (ImGui::BeginTable(
        "##items_loan_table",
        8, flags
        // ImVec2(0.f, TEXT_BASE_HEIGHT * (float)num_rows)
    )) {
        // ImGui::TableSetupColumn(
        //     "Actions",
        //     ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort,
        //     40.f);
        ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible

        ImGui::TableSetupColumn("Nom");
        ImGui::TableSetupColumn("Prénom");
        ImGui::TableSetupColumn("Date de naissance");
        ImGui::TableSetupColumn("Unité / Chambre");
        ImGui::TableSetupColumn("Remarque(s)", ImGuiTableColumnFlags_NoSort);
        ImGui::TableSetupColumn("Date d'emprunt", ImGuiTableColumnFlags_DefaultSort);
        ImGui::TableSetupColumn("Objet(s)");
        ImGui::TableHeadersRow();

        if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs()) {
            if (sorts_specs->SpecsDirty) {
                m_ascending = sorts_specs->Specs->SortDirection == ImGuiSortDirection_Ascending ? true : false;
                m_sort_id = (Cols)sorts_specs->Specs->ColumnIndex;
                m_fill_cols = true;
                sorts_specs->SpecsDirty = false;
            }
        }

        if (m_manager->isChanged(m_sub_id) || m_fill_cols) {
            fill_items();
        }

        if (m_ascending) {
            for (auto first_sort : m_loans) {
                for (auto second_sort : first_sort.second)
                    show_row(second_sort.second);
            }
        }
        else {
            for (auto it = m_loans.rbegin();it != m_loans.rend();it++) {
                for (auto second_sort : it->second)
                    show_row(second_sort.second);
            }
        }
        ImGui::EndTable();
    }
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor();
}