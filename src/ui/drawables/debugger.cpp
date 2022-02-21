#include "debugger.h"

namespace Debug {
    using namespace Item;

    void Debugger::findBugs() {
        m_manager = m_workspace.getCurrentManager();

        // Loan duplicates
        std::map<LoanID, ItemID> loans;
        m_duplicates.clear();
        for (auto pair : m_manager->m_item_loan_map) {
            for (auto loan_id : pair.second) {
                if (loans.contains(loan_id)) {
                    if (m_duplicates[loan_id].empty()) {
                        m_duplicates[loan_id].push_back(loans[loan_id]);
                    }
                    m_duplicates[loan_id].push_back(pair.first);
                }
                else {
                    loans[loan_id] = pair.first;
                }
            }
        }
    }

    void Debugger::item_info(ItemID item_id) {
        auto item = m_manager->getItem(item_id).value();
        auto cat = m_manager->getCategory(item->category).value();
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            for (auto prop_id : cat->properties) {
                auto prop = m_manager->getProperty(prop_id).value();
                Tempo::PushFont(m_ui_state->font_bold);
                ImGui::Text((prop->name + " ").c_str());
                Tempo::PopFont();
                ImGui::SameLine();
                ImGui::Text(item->property_values[prop_id].c_str());
            }
            ImGui::EndTooltip();
        }

    }
    void Debugger::person_info(PersonID person_id) {
        auto person = m_manager->getPerson(person_id).value();
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            std::string txt = person->surname + ", " + person->name + ", ";
            txt += person->birthday.format("%d/%m/%Y") + ", " + person->place;
            ImGui::Text(txt.c_str());
            ImGui::EndTooltip();
        }
    }

    void Debugger::FrameUpdate() {
        m_manager = m_workspace.getCurrentManager();
        if (!m_show)
            return;

        findBugs();

        ImGui::Begin("Debugguer", &m_show);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 4));

        if (m_duplicates.size()) {
            ImGui::Text(
                "Attention, cette fenêtre permet de modifier sans contrôle les données du logiciel.\n"
                "Si vous ne savez pas ce que vous faites, fermez cette fenêtre.");
            if (ImGui::Button("Sauvegarder changements")) {
                m_workspace.save("changements_debug");
            }
            ImGui::Separator();
            if (ImGui::TreeNode("Emprunts dupliqués")) {
                for (auto pair : m_duplicates) {
                    auto loan = m_manager->getLoan(pair.first).value();
                    std::string str;
                    str += "Loan" + std::to_string(loan->id) + " Emprunté le " + loan->date.format("%d/%m/%Y");
                    ImGui::Text(str.c_str());
                    person_info(loan->person);
                    ImGui::Indent(40.f);
                    for (auto item_id : pair.second) {
                        ImGui::Text(("Item" + std::to_string(item_id)).c_str());
                        item_info(item_id);
                        ImGui::SameLine();
                        if (ImGui::Button("Enlever")) {
                            m_manager->m_item_loan_map.erase(item_id);
                        }
                    }
                    ImGui::Unindent(40.f);
                }
                ImGui::TreePop();
            }
        }
        ImGui::PopStyleVar();
        ImGui::End();
    }
    void Debugger::BeforeFrameUpdate() {

    }
}