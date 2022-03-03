#include <random>

#include "debugger.h"
#include "ui/imgui_util.h"
#include "imgui_stdlib.h"

#include "core/names.h"

namespace Debug {
    using namespace Item;

    void Debugger::findBugs() {
        m_manager = m_workspace.getCurrentManager();

        // Loan duplicates
        std::map<LoanID, ItemID> loans_d;
        m_duplicates.clear();
        for (auto pair : m_manager->m_item_loan_map) {
            for (auto loan_id : pair.second) {
                if (loans_d.contains(loan_id)) {
                    if (m_duplicates[loan_id].empty()) {
                        m_duplicates[loan_id].push_back(loans_d[loan_id]);
                    }
                    m_duplicates[loan_id].push_back(pair.first);
                }
                else {
                    loans_d[loan_id] = pair.first;
                }
            }
        }
        m_discrepancies_item.clear();
        m_discrepancies_persons.clear();
        // Discrepancies
        auto loans = m_manager->getActiveLoans();
        auto loan_map = m_manager->m_item_loan_map;
        for (auto pair : m_manager->m_item_loan_map) {
            for (auto loan_id : pair.second) {
                auto loan = m_manager->getLoan(loan_id).value();
                if (!loans.contains(loan_id) || loan_id != loan->id) {
                    m_discrepancies_item.insert(pair.first);
                }
            }
        }
        for (auto pair : m_manager->m_person_loan_map) {
            for (auto loan_id : pair.second) {
                auto loan = m_manager->getLoan(loan_id).value();
                if (!loans.contains(loan_id) || loan_id != loan->id) {
                    m_discrepancies_persons.insert(pair.first);
                }
            }
        }
    }

    void Debugger::scramble_data() {
        auto names = getNames();
        auto surnames = getSurnames();
        std::vector<std::string> places;
        for (int i = 15; i <= 44;i++) {
            for (int j = 400;j <= 800;) {
                places.push_back(std::to_string(j + i));
                j += 100;
            }
        }
        for (int i = 0;i < 10;i++) {
            places.push_back("SIB");
            places.push_back("SIC");
        }

        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> names_dist(0, (int)names.size() - 1);
        std::uniform_int_distribution<std::mt19937::result_type> surnames_dist(0, (int)surnames.size() - 1);
        std::uniform_int_distribution<std::mt19937::result_type> day_dist(1, 31);
        std::uniform_int_distribution<std::mt19937::result_type> month_dist(1, 12);
        std::uniform_int_distribution<std::mt19937::result_type> year_dist(1920, 2001);
        std::uniform_int_distribution<std::mt19937::result_type> place_dist(0, (int)places.size() - 1);
        std::uniform_int_distribution<std::mt19937::result_type> date_dist(0, 2);

        // First replace all the names
        for (auto& pair : m_manager->m_registered_persons) {
            pair.second->name = names[(int)names_dist(rng)];
            pair.second->surname = surnames[(int)surnames_dist(rng)];
            Date date;
            while (!date.isValid()) {
                date = Date{
                    (uint8_t)day_dist(rng),
                    (uint8_t)month_dist(rng),
                    (uint16_t)year_dist(rng) };
            }
            pair.second->birthday = date;
            pair.second->place = places[place_dist(rng)];
        }
        // First replace all the names
        for (auto& pair : m_manager->m_retired_persons) {
            pair.second->name = names[(int)names_dist(rng)];
            pair.second->surname = surnames[(int)surnames_dist(rng)];
            Date date;
            while (!date.isValid()) {
                date = Date{
                    (uint8_t)day_dist(rng),
                    (uint8_t)month_dist(rng),
                    (uint16_t)year_dist(rng) };
            }
            pair.second->birthday = date;
            pair.second->place = places[place_dist(rng)];
        }
        int i = 0;
        for (auto& pair : m_manager->m_registered_loans) {
            if (i % 2 == 1)
                continue;
            Date date = pair.second->date;
            Date date_scrambled = Date{
                (uint8_t)((int)date.getDay() + date_dist(rng) - 1),
                (uint8_t)((int)date.getMonth() + date_dist(rng) - 1),
                date.getYear() };
            if (date_scrambled.isValid()) {
                pair.second->date = date_scrambled;
            }
            i++;
        }
    }

    void Debugger::item_info(ItemID item_id) {
        if (!m_manager->getItem(item_id).has_value()) {
            ImGui::BulletText(("Deleted item " + std::to_string(item_id)).c_str());
            return;
        }
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
        if (!m_manager->getPerson(person_id).has_value()) {
            ImGui::BulletText(("Deleted person" + std::to_string(person_id)).c_str());
            return;
        }
        auto person = m_manager->getPerson(person_id).value();
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            std::string txt = person->surname + ", " + person->name + ", ";
            txt += person->birthday.format("%d/%m/%Y") + ", " + person->place;
            ImGui::Text(txt.c_str());
            ImGui::EndTooltip();
        }
    }

    void Debugger::show_item(ItemID item_id, long long int counter) {
        bool ret = ImGui::TreeNode(labelize(item_id, "Item " + std::to_string(item_id), counter).c_str());
        if (ret) {
            if (!m_manager->getItem(item_id).has_value()) {
                ImGui::Text("Deleted item");
                ImGui::TreePop();
                return;
            }
            auto item = m_manager->getItem(item_id).value();
            auto cat = m_manager->getCategory(item->category).value();
            ImGui::Text("Catégorie:"); ImGui::SameLine();
            ImGui::Text(cat->name.c_str());
            for (auto prop_id : cat->properties) {
                auto prop = m_manager->getProperty(prop_id).value();
                Tempo::PushFont(m_ui_state->font_bold);
                ImGui::Text((prop->name + " ").c_str());
                Tempo::PopFont();
                ImGui::SameLine();
                ImGui::Text(item->property_values[prop_id].c_str());
            }
            ImGui::TreePop();
        }
    }

    void Debugger::show_person(PersonID person_id, long long int counter) {
        bool ret = ImGui::TreeNode(labelize(person_id, "Person " + std::to_string(person_id), counter).c_str());
        if (ret) {
            if (!m_manager->getPerson(person_id).has_value()) {
                ImGui::Text(("Deleted person " + std::to_string(person_id)).c_str());
                ImGui::TreePop();
                return;
            }
            auto person = m_manager->getPerson(person_id).value();
            std::string txt = person->surname + ", " + person->name + ", ";
            txt += person->birthday.format("%d/%m/%Y") + ", " + person->place;
            ImGui::Text(txt.c_str());
            ImGui::TreePop();
        }
    }

    void Debugger::show_loan(LoanID loan_id, long long int counter) {
        if (!m_manager->getLoan(loan_id).has_value()) {
            ImGui::Text(("Deleted loan " + std::to_string(loan_id)).c_str());
            return;
        }
        bool is_retired = m_manager->isRetired(loan_id).value();
        std::string str = "Loan " + std::to_string(loan_id);
        str += (is_retired) ? "(r)" : "(a)";
        bool ret = ImGui::TreeNode(labelize(loan_id, str, counter).c_str());
        if (ret) {
            if (ImGui::Button(labelize(loan_id, "Retire##Retire", counter).c_str())) {
                m_to_retire.insert(loan_id);
            }
            ImGui::SameLine();
            if (ImGui::Button(labelize(loan_id, "Remove##Remove", counter).c_str())) {
                m_to_remove.insert(loan_id);
            }
            auto loan = m_manager->getLoan(loan_id).value();
            show_item(loan->item, counter + 1);
            show_person(loan->person, counter + 2);
            ImGui::TreePop();
        }
    }


    void Debugger::FrameUpdate() {
        if (m_to_retire.size()) {
            for (auto loan_id : m_to_retire) {
                m_manager->retireLoan(loan_id, getCurrentDate());
            }
            m_to_retire.clear();
        }
        if (m_to_remove.size()) {
            for (auto loan_id : m_to_remove) {
                bool is_retired = m_manager->isRetired(loan_id).value();
                if (is_retired)
                    m_manager->m_retired_loans.erase(loan_id);
                else
                    m_manager->m_registered_loans.erase(loan_id);
            }
            m_to_remove.clear();
        }
        m_manager = m_workspace.getCurrentManager();
        if (!m_show)
            return;

        findBugs();

        ImGui::Begin("Debugguer", &m_show);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 4));

        ImGui::Text(
            "Attention, cette fenêtre permet de modifier sans contrôle les données du logiciel.\n"
            "Si vous ne savez pas ce que vous faites, fermez cette fenêtre.");
        if (ImGui::Button("Sauvegarder changements")) {
            m_workspace.save("changements_debug");
        }
        ImGui::InputText("Set global id", &m_global_id, ImGuiInputTextFlags_CharsDecimal);
        ImGui::SameLine();
        if (ImGui::Button("ok")) {
            double num = std::stod(m_global_id);
            Base::setID((int)num);
        }
        if (ImGui::Button("Mélanger et anonymiser données")) {
            scramble_data();
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

        if (ImGui::TreeNode("Emprunts incohérents")) {
            if (ImGui::TreeNode("Items")) {
                for (auto id : m_discrepancies_item) {
                    ImGui::Text(std::to_string(id).c_str());
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Persons")) {
                for (auto id : m_discrepancies_persons) {
                    ImGui::Text(std::to_string(id).c_str());
                }
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }

        ImGui::Separator();
        // Content
        long long int counter = 0;
        if (ImGui::TreeNode("item_loan_map")) {
            std::set<ItemID> to_remove;
            for (auto& pair : m_manager->m_item_loan_map) {
                if (ImGui::Button(labelize(pair.first, "Remove##remove", counter).c_str())) {
                    to_remove.insert(pair.first);
                }
                ImGui::SameLine();
                bool ret = ImGui::TreeNode(labelize(pair.first, std::to_string(pair.first), counter).c_str());
                item_info(pair.first);
                if (ret) {
                    std::set<LoanID> to_remove2;
                    for (auto loan_id : pair.second) {
                        if (ImGui::Button(labelize(loan_id, "Remove##remove2", counter).c_str())) {
                            to_remove2.insert(loan_id);
                        }
                        ImGui::SameLine();
                        show_loan(loan_id, counter);
                        counter++;
                    }
                    for (auto id : to_remove2) {
                        m_manager->m_item_loan_map[pair.first].erase(id);
                    }
                    ImGui::TreePop();
                }
                counter++;
            }
            for (auto id : to_remove) {
                m_manager->m_item_loan_map.erase(id);
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("person_loan_map")) {
            std::set<ItemID> to_remove;
            for (auto& pair : m_manager->m_person_loan_map) {
                if (ImGui::Button(labelize(pair.first, "Remove##remove", counter).c_str())) {
                    to_remove.insert(pair.first);
                }
                ImGui::SameLine();
                bool ret = ImGui::TreeNode(labelize(pair.first, std::to_string(pair.first), counter).c_str());
                person_info(pair.first);
                if (ret) {
                    std::set<LoanID> to_remove2;
                    for (auto loan_id : pair.second) {
                        if (ImGui::Button(labelize(loan_id, "Remove##remove2", counter).c_str())) {
                            to_remove2.insert(loan_id);
                        }
                        ImGui::SameLine();
                        show_loan(loan_id, counter);
                        counter++;
                    }
                    for (auto id : to_remove2) {
                        m_manager->m_person_loan_map[pair.first].erase(id);
                    }
                    ImGui::TreePop();
                }
                counter++;
            }
            for (auto id : to_remove) {
                m_manager->m_person_loan_map.erase(id);
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("registered_items")) {
            for (auto pair : m_manager->m_registered_items) {
                show_item(pair.first, counter);
                counter++;
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("registered_loans")) {
            for (auto pair : m_manager->m_registered_loans) {
                show_loan(pair.first, counter);
                counter++;
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("registered_persons")) {
            for (auto pair : m_manager->m_registered_persons) {
                show_person(pair.first, counter);
                counter++;
            }
            ImGui::TreePop();
        }
        ImGui::Separator();
        if (ImGui::TreeNode("past_item_loan_map")) {
            for (auto pair : m_manager->m_past_item_loan_map) {
                bool ret = ImGui::TreeNode(labelize(pair.first, std::to_string(pair.first), counter).c_str());
                item_info(pair.first);
                if (ret) {
                    for (auto loan_id : pair.second) {
                        show_loan(loan_id, counter);
                        counter++;
                    }
                    ImGui::TreePop();
                }
                counter++;
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("past_person_loan_map")) {
            for (auto pair : m_manager->m_past_person_loan_map) {
                bool ret = ImGui::TreeNode(labelize(pair.first, std::to_string(pair.first), counter).c_str());
                person_info(pair.first);
                if (ret) {
                    for (auto loan_id : pair.second) {
                        show_loan(loan_id, counter);
                        counter++;
                    }
                    ImGui::TreePop();
                }
                counter++;
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("retired_items")) {
            for (auto pair : m_manager->m_retired_items) {
                show_item(pair.first, counter);
                counter++;
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("retired_loans")) {
            for (auto pair : m_manager->m_retired_loans) {
                show_loan(pair.first, counter);
                counter++;
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("retired_persons")) {
            for (auto pair : m_manager->m_retired_persons) {
                show_person(pair.first, counter);
                counter++;
            }
            ImGui::TreePop();
        }
        ImGui::PopStyleVar();
        ImGui::End();
    }
    void Debugger::BeforeFrameUpdate() {

    }
}