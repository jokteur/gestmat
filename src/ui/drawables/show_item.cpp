#include "show_item.h"
#include "ui/widgets/misc.h"
#include "ui/imgui_util.h"
#include "ui/widgets/modal.h"

ShowItem::ShowItem(UIState_ptr ui_state) : Drawable(ui_state) {
    m_listener.filter = "change_manager";
    m_listener.callback = [this](Tempo::Event_ptr event) {
        m_item = nullptr;
        m_show = false;
    };
    Tempo::EventQueue::getInstance().subscribe(&m_listener);
}
ShowItem::~ShowItem() {
    Tempo::EventQueue::getInstance().unsubscribe(&m_listener);
}

bool ShowItem::isOpen() {
    return !(m_item == nullptr || !m_show);
}

void ShowItem::show_infos() {
    auto cat = m_manager->getCategory(m_item->category).value();
    for (auto prop_id : cat->properties) {
        auto prop = m_manager->getProperty(prop_id).value();
        Tempo::PushFont(m_ui_state->font_bold);
        if (!m_item->property_values.contains(prop_id))
            m_item->property_values[prop_id] = "";
        ImGui::Text(prop->name.c_str());
        Tempo::PopFont();
        ImGui::SameLine();
        ImGui::Text((": " + m_item->property_values[prop_id]).c_str());
    }
}

void ShowItem::show_notes() {
    std::set<Item::NoteID> to_remove;
    float width = ImGui::GetContentRegionAvail().x;
    size_t num_notes = m_item->notes.size();
    if (num_notes)
        ImGui::BeginChild("notes_left", ImVec2(width * 0.45f, 0));
    for (auto it = m_item->notes.rbegin();it != m_item->notes.rend();it++) {
        Tempo::PushFont(m_ui_state->font_bold);
        timestampToText(it->timestamp);
        Tempo::PopFont();
        ImGui::AlignTextToFramePadding();
        ImGui::Text(it->content.c_str());
        ImGui::SameLine();
        if (button(labelize(it->id, "Supprimer"), m_ui_state)) {
            to_remove.insert(it->id);
        }
        ImGui::Separator();
    }
    for (auto it = m_item->notes.begin();it != m_item->notes.end();it++) {
        if (to_remove.contains(it->id)) {
            m_item->notes.erase(it);
            m_workspace.save("enlever_note");
            break;
        }
    }
    if (num_notes) {
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("notes_right", ImVec2(width * 0.45f, 0));
    }
    if (m_new_note) {
        ImGui::SameLine();
        if (button("Annuler##new_note_cancel", m_ui_state)) {
            m_new_note = false;
        }
        ImGui::SameLine();
        if (button("Ajouter##new_note_add", m_ui_state)) {
            if (!m_note.empty()) {
                Item::Note note{
                    .content = m_note,
                    .timestamp = getTimestamp()
                };
                m_item->notes.push_back(note);
            }
            m_note = "";
            m_new_note = false;
            m_workspace.save("ajouter_note");
        }
        float height = ImGui::GetContentRegionAvail().y;
        ImGui::InputTextMultiline("##new_note_input", &m_note, ImVec2(0, height));
    }
    else {
        if (button("Nouvelle note##new_note_plus", m_ui_state)) {
            m_new_note = true;
            m_note = "";
        }
    }
    if (num_notes)
        ImGui::EndChild();
}

void ShowItem::person_widget(Item::Person_ptr person) {
    Tempo::PushFont(m_ui_state->font_bold);
    ImGui::Text((person->surname + " " + person->name).c_str());
    Tempo::PopFont();
    ImGui::SameLine();
    std::string txt = ", " + person->birthday.format("%d/%m/%Y");
    txt += ", " + person->place;
    ImGui::Text(txt.c_str());
}

void ShowItem::show_history() {
    auto retired_loans = m_manager->findRetiredLoans(m_item->id);
    auto loans = m_manager->findLoans(m_item->id);
    if (loans.has_value() && loans.value().size()) {
        ImGui::Text("Actuellement en emprunt chez");
        ImGui::SameLine();

        auto it = loans.value().begin();
        auto loan = m_manager->getLoan(*it).value();
        auto person = m_manager->getPerson(loan->person).value();
        person_widget(person);
    }
    if (!retired_loans.has_value() && !loans.has_value()) {
        ImGui::Text("Aucun emprunt trouvé");
    }
    if (retired_loans.has_value()) {
        std::map<std::string, std::vector<Item::Loan_ptr>> ordered_loans;
        auto rloans = retired_loans.value();
        for (auto it = rloans.begin();it != rloans.end();it++) {
            auto loan = m_manager->getLoan(*it).value();
            ordered_loans[loan->date.format("%Y%m%d")].push_back(loan);
        }
        for (auto it = ordered_loans.rbegin();it != ordered_loans.rend();it++) {
            for (auto loan : it->second) {
                auto person = m_manager->getPerson(loan->person).value();
                // ImGui::Indent(40.f);
                ImGui::Text(("Emprunté le " + loan->date.format("%d/%m/%Y")).c_str());
                ImGui::SameLine();
                ImGui::Text("par");
                ImGui::SameLine();
                person_widget(person);
                ImGui::Text(("Rendu le " + loan->date_back.format("%d/%m/%Y")).c_str());
                ImGui::Separator();
                // ImGui::Unindent(40.f);
            }
        }
    }
}

void ShowItem::retire_item() {
    const modal_fct error_fct = [this](bool& show, bool&, bool&) {
        ImGui::Text("Vous êtes sur de retirer un objet.");
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.f);
        ImGui::Text("Cette action va rendre tous les emprunts liés à cet objet, mais il ne va pas être supprimé.\n"
            "Un objet retiré ne peut plus être emprunté. Il est possible de restaurer les objets par la suite.");

        if (button(labelize(m_item->id, "Annuler"), m_ui_state)) {
            show = false;
        }
        ImGui::SameLine();
        if (button(labelize(m_item->id, "Retirer"), m_ui_state)) {
            m_manager->retireItem(m_item->id);
            show = false;
            m_workspace.save("retirer_objet");
        }
    };
    Modals::getInstance().setModal("Voulez-vous retirer un objet ?", error_fct);

}
void ShowItem::delete_item() {
    const modal_fct error_fct = [this](bool& show, bool&, bool&) {
        ImGui::Text("Vous êtes sur de supprimer un objet.");
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.f);
        ImGui::Text("Cette action va rendre supprimer l'objet et ne pourra plus être restauré.\n");

        if (button(labelize(m_item->id, "Annuler"), m_ui_state)) {
            show = false;
        }
        ImGui::SameLine();
        if (button(labelize(m_item->id, "Supprimer"), m_ui_state)) {
            m_manager->deleteRetiredItem(m_item->id);
            show = false;
            m_workspace.save("supprimer_objet");
        }
    };
    Modals::getInstance().setModal("Voulez-vous retirer un objet ?", error_fct);
}
void ShowItem::unretire_item() {
    m_manager->unretireItem(m_item->id);
    m_workspace.save("restaurer_objet");
}

void ShowItem::show_actions() {
    bool is_retired = m_manager->isRetired(m_item->id).value();
    if (is_retired) {
        if (button(labelize(m_item->id, "Restaurer"), m_ui_state)) {
            unretire_item();
        }
        ImGui::SameLine();
        if (button(labelize(m_item->id, "Supprimer"), m_ui_state, "", ImVec4(0.8f, 0.1f, 0.1f, 0.7f))) {
            delete_item();
        }
    }
    else {
        ImGui::Text("Objet perdu ou en réparation ?");
        if (button(labelize(m_item->id, "Retirer"), m_ui_state, "", ImVec4(0.8f, 0.1f, 0.1f, 0.7f))) {
            retire_item();
        }
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text(
                "L'action 'retirer' enlève un objet de la liste des objets empruntables.\n"
                "Cela ne supprime pas directement l'objet. Pour pouvoir supprimer un objet\n"
                "il faut d'abord le retirer puis le supprimer."
            );
            ImGui::EndTooltip();
        }
    }
}

void ShowItem::FrameUpdate() {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 5));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 40.f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.5f, 0.5f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.7f, 0.7f, 0.5f));
    if (ImGui::Button("X")) {
        m_show = false;
        m_just_closed = true;
    }
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);

    m_manager = m_workspace.getCurrentManager();
    if (m_manager == nullptr)
        return;
    if (m_item == nullptr)
        return;
    ImGui::SameLine();
    auto cat = m_manager->getCategory(m_item->category).value();
    ImGui::Text("Objet sélectionné dans");
    Tempo::PushFont(m_ui_state->font_italic);
    ImGui::SameLine();
    ImGui::Text(cat->name.c_str());
    Tempo::PopFont();

    ImGui::BeginTabBar("show_item_bar");
    if (ImGui::BeginTabItem("Infos")) {
        m_selected_tab = INFOS;
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Notes")) {
        m_selected_tab = NOTES;
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Historique")) {
        m_selected_tab = HISTORY;
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Actions")) {
        m_selected_tab = ACTIONS;
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();

    ImGui::BeginChild("show_item_child");
    if (m_selected_tab == INFOS) {
        show_infos();
    }
    else if (m_selected_tab == NOTES) {
        show_notes();
    }
    else if (m_selected_tab == HISTORY) {
        show_history();
    }
    else if (m_selected_tab == ACTIONS) {
        show_actions();
    }
    ImGui::EndChild();
}
void ShowItem::BeforeFrameUpdate() {

}

bool ShowItem::justClosed() {
    if (m_just_closed) {
        m_just_closed = false;
        return true;
    }
    return false;
}

void ShowItem::setItem(Item::Item_ptr item) {
    m_show = true;
    m_item = item;
}