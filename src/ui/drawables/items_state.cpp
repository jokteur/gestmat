#include "items_state.h"

#include "ui/widgets/misc.h"
#include "ui/imgui_util.h"
#include "ui/widgets/modal.h"

void ItemsState::BeforeFrameUpdate() {
}


ItemsState::ItemsState(UIState_ptr ui_state) :
    Drawable(ui_state), m_filter(ui_state, "Rechercher (nom, prénom, date, infos objet, chambre, ...)") {
}

void ItemsState::fill_items() {
    m_loans.clear();
    for (auto loan_id : m_manager->getActiveLoans()) {
        auto loan = m_manager->getLoan(loan_id).value();
        auto item = m_manager->getItem(loan->item).value();
        auto cat = m_manager->getCategory(item->category).value();
        auto person = m_manager->getPerson(loan->person).value();

        Filter filter;
        filter.insert(core::toLower(person->name));
        filter.insert(core::toLower(person->surname));
        filter.insert(core::toLower(person->place));
        filter.insert(core::toLower(person->birthday.format("%Y/%m/%d")));
        filter.insert(core::toLower(loan->date.format("%Y/%m/%d")));
        filter.insert(core::toLower(cat->name));
        for (auto pair : item->property_values) {
            if (cat->properties_hide.contains(pair.first)) {
                continue;
            }
            filter.insert(core::toLower(pair.second));
        }

        auto make_pair = [=](Item::Loan_ptr loa) {
            return std::make_pair(filter, loa);
        };

        std::string key = person->surname + person->name;
        if (m_sort_id == TYPE) {
            m_loans[cat->name][key].push_back(make_pair(loan));
        }
        else if (m_sort_id == SURNAME) {
            m_loans[person->surname][key].push_back(make_pair(loan));
        }
        else if (m_sort_id == NAME) {
            m_loans[person->name][key].push_back(make_pair(loan));
        }
        else if (m_sort_id == BIRTHDAY) {
            m_loans[person->birthday.format("%Y/%m/%d")][key].push_back(make_pair(loan));
        }
        else if (m_sort_id == PLACE) {
            m_loans[person->place][key].push_back(make_pair(loan));
        }
        else {
            m_loans[loan->date.format("%Y/%m/%d")][key].push_back(make_pair(loan));
        }
    }
}


void ItemsState::show_row(std::vector<std::pair<Filter, Item::Loan_ptr>> loans) {
    ImGui::TableNextRow();

    // std::cout << loans.size() << std::endl;

    auto person = m_manager->getPerson(loans[0].second->person).value();
    // std::cout << person->name << std::endl;
    ImGui::TableSetColumnIndex(SURNAME);
    ImGui::Text(person->surname.c_str());

    ImGui::TableSetColumnIndex(NAME);
    ImGui::Text(person->name.c_str());

    ImGui::TableSetColumnIndex(BIRTHDAY);
    ImGui::Text(person->birthday.format("%d/%m/%Y").c_str());

    ImGui::TableSetColumnIndex(PLACE);
    ImGui::Text(person->place.c_str());

    ImGui::TableSetColumnIndex(NOTES);
    for (auto& note : person->notes) {
        Tempo::PushFont(m_ui_state->font_bold);
        timestampToText(note.timestamp);
        Tempo::PopFont();
        ImGui::Text(note.content.c_str());
    }

    // Recalculate filter
    std::map<Item::Loan_ptr, bool> pass_filter;
    for (auto loan_pair : loans) {
        pass_filter[loan_pair.second] = m_filter.PassFilter(loan_pair.first);
    }

    int i = 0;
    Tempo::PushFont(m_ui_state->font_bold);
    for (auto loan_pair : loans) {
        auto loan = loan_pair.second;
        if (!pass_filter[loan])
            continue;
        ImGui::TableSetColumnIndex(LOAN_DATE);
        if (!i)
            ImGui::Text("");
        ImGui::Text(loan->date.format("%d/%m/%Y").c_str());
        i++;
    }
    Tempo::PopFont();
    i = 0;

    ImGui::TableSetColumnIndex(TYPE);
    for (auto loan_pair : loans) {
        auto loan = loan_pair.second;
        if (!pass_filter[loan])
            continue;

        Item::Item_ptr item = m_manager->getItem(loan->item).value();
        Item::Category_ptr cat = m_manager->getCategory(item->category).value();

        if (!i)
            ImGui::Text("");
        // Infos box for items
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
        // 
        ImGui::Selectable(labelize(loan->id, cat->name, cat->id).c_str(), &m_loans_checkbox[loan->id]);
        i++;
    }
    ImGui::TableSetColumnIndex(ACTIONS);
    i = 0;
    for (auto loan_pair : loans) {
        auto loan = loan_pair.second;
        if (!pass_filter[loan])
            continue;
        if (!i)
            ImGui::Text("");
        ImGui::Checkbox(labelize(loan->id, "##checkbox").c_str(), &m_loans_checkbox[loan->id]);
        i++;
    }
}

void ItemsState::give_back() {
    std::vector<Item::LoanID> to_delete;
    for (auto pair : m_loans_checkbox) {
        if (pair.second) {
            m_manager->retireLoan(pair.first, getCurrentDate());
            to_delete.push_back(pair.first);
        }
    }
    for (auto it : to_delete) {
        m_loans_checkbox.erase(it);
    }

    m_workspace.save("rendre_objet");
}

void ItemsState::FrameUpdate() {
    m_manager = m_workspace.getCurrentManager();
    if (m_sub_id == 0) {
        m_sub_id = m_manager->getId();
    }
    title("Emprunts en cours", m_ui_state);

    m_filter.FrameUpdate();

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Actions:");
    ImGui::SameLine();
    std::string label = "Rendre les objets sélectionnés";
    std::string err = "Veuillez sélectionner au min. 1 objet (dans la colonne à droite)";

    int num = 0;
    for (auto pair : m_loans_checkbox) {
        if (pair.second)
            num++;
    }
    if (num > 0) {
        label += " (" + std::to_string(num) + ")";
        err = "";
    }
    label += "##give_back_button";
    if (button(label, m_ui_state, err)) {
        give_back();
    }

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
    )) {
        ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible

        ImGui::TableSetupColumn("Nom");
        ImGui::TableSetupColumn("Prénom");
        ImGui::TableSetupColumn("Date de naissance");
        ImGui::TableSetupColumn("Unité / Chambre");
        ImGui::TableSetupColumn("Remarque(s)", ImGuiTableColumnFlags_NoSort);
        ImGui::TableSetupColumn("Date d'emprunt", ImGuiTableColumnFlags_DefaultSort);
        ImGui::TableSetupColumn("Objet(s)");
        ImGui::TableSetupColumn(" ", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, 35.f);
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
                for (auto second_sort : first_sort.second) {
                    bool draw = false;
                    for (auto pair : second_sort.second) {
                        if (m_filter.PassFilter(pair.first)) {
                            draw = true;
                            break;
                        }
                    }
                    if (draw)
                        show_row(second_sort.second);
                }
            }
        }
        else {
            for (auto it = m_loans.rbegin();it != m_loans.rend();it++) {
                for (auto second_sort : it->second) {
                    bool draw = false;
                    for (auto pair : second_sort.second) {
                        if (m_filter.PassFilter(pair.first)) {
                            draw = true;
                            break;
                        }
                    }
                    if (draw)
                        show_row(second_sort.second);
                }
            }
        }
        ImGui::EndTable();
    }
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor();
}