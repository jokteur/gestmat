#include "properties_widget.h"

#include "ui/widgets/misc.h"
#include "ui/widgets/modal.h"
#include "ui/imgui_util.h"

PropertiesWidget::PropertiesWidget(UIState_ptr ui_state) : Drawable(ui_state) {

}

Property::Property(UIState_ptr ui_state, Item::PropertyID prop_id) : Drawable(ui_state), m_prop_id(prop_id) {
    m_manager = m_workspace.getCurrentManager();
    if (prop_id != -1) {
        m_prop = m_manager->getProperty(prop_id).value();
        m_name = m_prop->name;
        if (!m_prop->select.empty()) {
            m_choice = 1;
            m_select = m_prop->select;
        }
    }
}

void Property::BeforeFrameUpdate() {
    m_manager = m_workspace.getCurrentManager();
    m_same_name = false;
    for (auto prop_id : m_manager->getAllProperties()) {
        if (prop_id == m_prop_id)
            continue;
        auto prop = m_manager->getProperty(prop_id).value();
        if (prop->name == m_name) {
            m_same_name = true;
        }
    }
}

void Property::save() {
    if (!m_choice) {
        m_select.clear();
    }
    if (m_prop_id == -1) {
        Item::Property prop;
        prop.name = m_name;
        prop.select = m_select;
        m_manager->createProperty(prop);
        m_workspace.save("nouvelle_propriete");
    }
    else {
        m_prop->name = m_name;
        m_prop->select = m_select;
        m_workspace.save("editer_propriete");
    }
}

void Property::FrameUpdate() {
    m_manager = m_workspace.getCurrentManager();

    std::string name = m_name;
    if (m_prop_id == -1 && name.empty()) {
        name = "Nouvelle propriété";
    }
    if (ImGui::CollapsingHeader(labelize(m_prop_id, name).c_str()), ImGuiTreeNodeFlags_DefaultOpen) {
        std::string error;
        if (m_same_name) {
            error = "Nom existe déjà";
        }
        labeledTextInput(&m_name, "Nom:", labelize(m_prop_id, "##prop_name"), "Entrez le nom", error);

        Tempo::PushFont(m_ui_state->font_bold);
        ImGui::Text("Type:");
        Tempo::PopFont();

        ImGui::SameLine();
        ImGui::RadioButton("libre", &m_choice, 0); ImGui::SameLine();
        ImGui::RadioButton("liste à choix", &m_choice, 1);

        if (m_choice) {
            int j = 0;
            std::vector<std::vector<std::string>::iterator> to_remove;
            for (auto it = m_select.begin();it != m_select.end(); it++) {
                ImGui::InputText(labelize(m_prop_id, "##input", j).c_str(), &(*it)); ImGui::SameLine();
                if (button(labelize(m_prop_id, "-##remove", j), m_ui_state)) {
                    to_remove.push_back(it);
                }
                j++;
            }
            for (auto it : to_remove) {
                m_select.erase(it);
            }
            if (button(labelize(m_prop_id, "+"), m_ui_state)) {
                m_select.push_back("");
            }
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("Ajouter un choix");
                ImGui::EndTooltip();
            }

        }

        if (button(labelize(m_prop_id, "Annuler"), m_ui_state)) {
            m_destroy_me = true;
        }
        ImGui::SameLine();

        std::string save_error;
        if (m_name.empty() || m_same_name) {
            save_error = "Veuillez remplir correctement les champs avant de sauvegarder";
        }
        if (button(labelize(m_prop_id, "Sauvegarder"), m_ui_state, save_error)) {
            m_destroy_me = true;
            save();
        }
    }
}

void PropertiesWidget::delete_warning(Item::Property_ptr prop) {
    const modal_fct error_fct = [this, prop](bool& show, bool&, bool&) {
        ImGui::Text("Vous êtes sur le point de supprimer la propriété ");
        Tempo::PushFont(m_ui_state->font_bold);
        ImGui::Text(prop->name.c_str());
        Tempo::PopFont();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.f);
        ImGui::Text("Cette action va supprimer la propriété définitivement et ne sera plus disponible dans la liste des objets.\n"
            "Pour continuer, veuillez taper le nom de la propriété dans le champ suivant:");
        ImGui::InputTextWithHint("##delete_prop", "Nom de la propriété à supprimer", &m_delete);

        if (m_delete == prop->name) {
            m_delete_error = "";
        }
        else {
            m_delete_error = "Il faut taper le nom pour continuer";
        }
        if (button(labelize(prop->id, "Annuler"), m_ui_state, "")) {
            show = false;
        }
        ImGui::SameLine();
        if (button(labelize(prop->id, "Supprimer"), m_ui_state, m_delete_error)) {
            m_manager->retireProperty(prop->id);
            m_manager->deleteRetiredProperty(prop->id);
            show = false;
            m_workspace.save("supprimer_propriete");
        }
    };
    m_delete = "";
    Modals::getInstance().setModal("Voulez-vous supprimer la propriété ?", error_fct);
}

void PropertiesWidget::FrameUpdate() {
    if (button("Retour", m_ui_state, "", ImVec4(), true)) {
        m_ui_state->active_panel = UIState::MANAGEMENT;
    }
    ImGui::SameLine();
    title("Éditer les propriétés pour les catégories", m_ui_state);
    if (button("+##new_property", m_ui_state)) {
        m_edit_widgets[-1] = std::make_shared<Property>(m_ui_state, -1);
    }
    if (m_edit_widgets.contains(-1)) {
        m_edit_widgets[-1]->FrameUpdate();
    }
    for (auto prop_id : m_manager->getAllProperties()) {
        auto prop = m_manager->getProperty(prop_id).value();
        if (m_edit_widgets.contains(prop_id)) {
            m_edit_widgets[prop_id]->FrameUpdate();
        }
        else {
            if (ImGui::CollapsingHeader(labelize(prop_id, prop->name).c_str())) {
                ImGui::Indent(40.f);
                if (button(labelize(prop_id, "Éditer"), m_ui_state)) {
                    m_edit_widgets[prop_id] = std::make_shared<Property>(m_ui_state, prop_id);
                }
                ImGui::SameLine();
                if (button(labelize(prop_id, "Supprimer"), m_ui_state, "", ImVec4(0.7f, 0.2f, 0.2f, 0.4f))) {
                    delete_warning(prop);
                }
                Tempo::PushFont(m_ui_state->font_bold);
                ImGui::Text("Type: ");
                Tempo::PopFont();
                ImGui::SameLine();
                if (prop->select.empty()) {
                    ImGui::Text("libre");
                }
                else {
                    ImGui::Text("liste à choix (choix:");
                    ImGui::SameLine();
                    std::string txt;
                    int i = 0;
                    for (auto& str : prop->select) {
                        if (i != 0) {
                            txt += ", ";
                        }
                        i++;
                        txt += str;
                    }
                    Tempo::PushFont(m_ui_state->font_italic);
                    ImGui::Text(txt.c_str());
                    Tempo::PopFont();
                    ImGui::SameLine();
                    ImGui::Text(")");
                }

                Tempo::PushFont(m_ui_state->font_bold);
                ImGui::Text("Utilisée dans les catégories suivantes:");
                Tempo::PopFont();
                std::vector<std::string> categories;
                for (auto cat_id : m_manager->getAllCategories()) {
                    auto cat = m_manager->getCategory(cat_id).value();
                    for (auto prop2_id : cat->properties) {
                        if (prop2_id == prop_id) {
                            categories.push_back(cat->name);
                            break;
                        }
                    }
                }
                for (auto& str : categories) {
                    ImGui::BulletText(str.c_str());
                }
                ImGui::Unindent(40.f);
            }
        }
    }
}

void PropertiesWidget::BeforeFrameUpdate() {
    m_manager = m_workspace.getCurrentManager();

    std::set<Item::PropertyID> to_destroy;
    for (auto widget : m_edit_widgets) {
        widget.second->BeforeFrameUpdate();
        if (widget.second->destroy_me()) {
            to_destroy.insert(widget.first);
        }
    }
    for (auto prop_id : to_destroy) {
        m_edit_widgets.erase(prop_id);
    }
}