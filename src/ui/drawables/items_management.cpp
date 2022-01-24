#include "items_management.h"

#include "ui/widgets/misc.h"
#include "ui/imgui_util.h"
#include "ui/widgets/modal.h"
#include "imgui_stdlib.h"

void ItemsManagement::BeforeFrameUpdate() {
    std::set<Item::CategoryID> to_destroy;
    for (auto& pair : m_cat_widgets) {
        if (pair.second != nullptr) {
            pair.second->BeforeFrameUpdate();
            if (pair.second->destroy_me()) {
                to_destroy.insert(pair.first);
            }
        }
    }
    for (auto id : to_destroy) {
        m_cat_widgets.erase(id);
    }
}

void ItemsManagement::FrameUpdate() {
    auto manager = m_workspace.getCurrentManager();

    ImGui::AlignTextToFramePadding();
    title("Catégories d'objets", m_ui_state);
    ImGui::SameLine();
    if (button("+##new_cat", m_ui_state)) {
        m_new_cat = true;
        m_cat_widgets[-1] = std::make_shared<CategoryWidget>(m_ui_state);
    }
    ImGui::SameLine();
    if (button("Propriétés##prop_edit", m_ui_state)) {

    }

    if (m_cat_widgets.contains(-1)) {
        m_cat_widgets[-1]->FrameUpdate();
    }


    for (auto cat_id : manager->getAllCategories()) {
        auto cat = manager->getCategory(cat_id).value();
        if (m_cat_widgets.contains(cat_id)) {
            m_cat_widgets[cat_id]->FrameUpdate();
        }
        else {
            if (ImGui::CollapsingHeader(labelize(cat_id, cat->name).c_str())) {
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Actions sur catégorie:");
                ImGui::SameLine();
                if (button("Éditer", m_ui_state)) {
                    m_cat_widgets[cat_id] = std::make_shared<CategoryWidget>(m_ui_state, cat_id);
                }
                ImGui::SameLine();
                if (button("Retirer", m_ui_state, "", ImVec4(0.7f, 0.2f, 0.2f, 0.4f))) {
                    const modal_fct error_fct = [this, cat, cat_id, manager](bool& show, bool&, bool&) {
                        ImGui::Text("Vous êtes sur le point de retirer la catégorie ");
                        ImGui::SameLine();
                        Tempo::PushFont(m_ui_state->font_italic);
                        ImGui::Text(cat->name.c_str());
                        Tempo::PopFont();
                        ImGui::Text("Cette action va cacher la catégorie de la liste et tout objet emprunté\n"
                            "de cette catégorie va être considéré comme rendu.\n"
                            "Pour continuer, veuillez taper le nom de la catégorie dans le champ suivant:");
                        ImGui::InputTextWithHint("", "Nom de la catégorie à supprimer", &m_retire);

                        if (m_retire == cat->name) {
                            m_retire_error = "";
                        }
                        else {
                            m_retire_error = "Il faut taper le nom pour continuer";
                        }
                        if (button(labelize(cat_id, "Annuler").c_str(), m_ui_state, "")) {
                            show = false;
                        }
                        ImGui::SameLine();
                        if (button(labelize(cat_id, "Retirer").c_str(), m_ui_state, m_retire_error)) {
                            manager->retireCategory(cat_id);
                            show = false;
                        }
                    };
                    ImGui::SetNextWindowSizeConstraints(ImVec2(400, 200), ImVec2(FLT_MAX, FLT_MAX));
                    Modals::getInstance().setModal("Voulez-vous retirer la catégorie ?", error_fct);
                }
            }
        }
    }
}