#include "category_widget.h"

#include "ui/widgets/misc.h"
#include "ui/imgui_util.h"
#include "imgui_stdlib.h"

void CategoryWidget::BeforeFrameUpdate() {
    m_manager = m_workspace.getCurrentManager();

    m_same_name = false;
    for (auto cat_id : m_manager->getAllCategories()) {
        auto cat = m_manager->getCategory(cat_id).value();
        if (m_category != nullptr) {
            if (m_name == cat->name && m_name != m_category->name) {
                m_same_name = true;
                break;
            }
        }
        else {

            if (m_name == cat->name) {
                m_same_name = true;
                break;
            }
        }
    }

    if (m_prev_name != m_name) {
        m_prev_name = m_name;
        Tempo::EventQueue::getInstance().post(Tempo::Event_ptr(new Tempo::Event("Tempo/redraw")));
    }
}

CategoryWidget::CategoryWidget(UIState_ptr ui_state, Item::CategoryID cat_id) : Drawable(ui_state), m_cat_id(cat_id) {
    m_manager = m_workspace.getCurrentManager();

    if (cat_id != -1) {
        m_category = m_manager->getCategory(cat_id).value();
        m_name = m_category->name;
        m_current_properties_order = m_category->properties;
        for (auto prop_id : m_current_properties_order) {
            m_current_properties.insert(prop_id);
        }
    }
}

void CategoryWidget::save() {
    if (m_category != nullptr) {
        m_category->name = m_name;
        m_category->properties = m_current_properties_order;
        m_workspace.save("editer_categorie", m_manager);
    }
    else {
        Item::Category category{
            .name = m_name,
            .properties = m_current_properties_order,
        };
        m_manager->createCategory(category);
        m_workspace.save("nouvelle_categorie", m_manager);
    }
    m_destroy = true;
}

void CategoryWidget::PropertiesWidget() {
    Tempo::PushFont(m_ui_state->font_bold);
    ImGui::Text("Propriétés");
    Tempo::PopFont();
    if (m_current_properties.empty()) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.5f, 0.f, 0.f, 0.7f), "(au minimum 1)");
    }
    ImGui::SameLine();
    help("<<< pour ajouter une propriété, >>> pour enlever\nGlissez & déposez dans la colonne à gauche pour changer l'ordre");

    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);

    float height = 300.0f;

    {
        ImGui::BeginChild(labelize(m_cat_id, "left_prop").c_str(), ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, height), true);
        std::set<int> to_remove;
        for (int n = 0;n < m_current_properties_order.size();n++) {
            auto prop_id = m_current_properties_order[n];
            auto prop = m_manager->getProperty(prop_id).value();

            float button_size = ImGui::CalcTextSize(">>>").x + ImGui::GetStyle().FramePadding.x * 3;

            ImVec2 frame_padding = ImGui::GetStyle().FramePadding;
            ImVec2 text_size = ImGui::CalcTextSize(prop->name.c_str());
            ImGui::Selectable(
                prop->name.c_str(),
                false,
                ImGuiSelectableFlags_DrawHoveredWhenHeld,
                ImVec2(ImGui::GetColumnWidth() - button_size - 10, frame_padding.y * 2 + text_size.y)
            );
            if (ImGui::IsItemActive() && !ImGui::IsItemHovered()) {
                int n_next = n + (ImGui::GetMouseDragDelta(0).y < 0.f ? -1 : 1);
                if (n_next >= 0 && n_next < m_current_properties_order.size()) {
                    auto temp_id = m_current_properties_order[n];
                    m_current_properties_order[n] = m_current_properties_order[n_next];
                    m_current_properties_order[n_next] = temp_id;
                    ImGui::ResetMouseDragDelta();
                }
            }
            ImGui::SameLine();

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - button_size);
            if (button(labelize(m_cat_id, ">>>", prop_id), m_ui_state)) {
                to_remove.insert(n);
                m_current_properties.erase(prop_id);
            }
            // Little space at the end
            // ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
        }
        ImGui::EndChild();
        for (auto n : to_remove) {
            m_current_properties_order.erase(m_current_properties_order.begin() + n);
        }
    }
    ImGui::SameLine();
    {
        ImGui::BeginChild(labelize(m_cat_id, "right_prop").c_str(), ImVec2(0, height), true);
        auto props = m_manager->getAllProperties();
        for (auto prop_id : props) {
            if (m_current_properties.contains(prop_id)) {
                continue;
            }
            auto prop = m_manager->getProperty(prop_id).value();
            ImGui::AlignTextToFramePadding();
            if (button(labelize(m_cat_id, "<<<", prop_id), m_ui_state)) {
                m_current_properties.insert(prop_id);
                m_current_properties_order.insert(m_current_properties_order.begin(), prop_id);
            }
            ImGui::SameLine();
            ImGui::Text(prop->name.c_str());
        }
        ImGui::EndChild();
    }
    ImGui::PopStyleVar();
}

void CategoryWidget::FrameUpdate() {
    std::string name = m_name;
    if (name.empty() && m_cat_id == -1) {
        name = "Nouvelle catégorie";
    }

    if (ImGui::CollapsingHeader(labelize(m_cat_id, name).c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
        std::string error;
        if (m_same_name)
            error = "Nom existe déjà";
        labeledTextInput(&m_name, "Nom:", std::string("##cat_name") + std::to_string(m_cat_id), "Entrez le nom", error);
        std::string save_error;
        if (m_same_name || m_name.empty() || m_current_properties.empty()) {
            save_error = "Veuillez remplir correctement les champs avant de sauvegarder";
        }

        // Properties
        PropertiesWidget();

        ImGui::Separator();

        float button_size = ImGui::CalcTextSize("Sauvegarder").x + ImGui::GetStyle().FramePadding.x * 3;
        float button_size2 = ImGui::CalcTextSize("Annuler").x + ImGui::GetStyle().FramePadding.x * 3;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - button_size - button_size2);

        if (button(labelize(m_cat_id, "Annuler"), m_ui_state)) {
            m_destroy = true;
        }
        ImGui::SameLine();
        if (button(labelize(m_cat_id, "Sauvegarder"), m_ui_state, save_error)) {
            save();
        }
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetTextLineHeight());
    }
}
