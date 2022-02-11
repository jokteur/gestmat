#include "items_management.h"

#include "ui/widgets/misc.h"
#include "ui/imgui_util.h"
#include "ui/widgets/modal.h"
#include "imgui_stdlib.h"

ItemsManagement::ItemsManagement(UIState_ptr ui_state) : Drawable(ui_state), m_show_item_widget(ui_state) {
    m_listener.filter = "change_manager";
    m_listener.callback = [this](Tempo::Event_ptr event) {
        m_manager = m_workspace.getCurrentManager();
        m_cat_widgets.clear();
        m_items_widgets.clear();
    };

    Tempo::EventQueue::getInstance().subscribe(&m_listener);
}
ItemsManagement::~ItemsManagement() {

    Tempo::EventQueue::getInstance().unsubscribe(&m_listener);
}

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
    for (auto& pair : m_cat_widgets) {
        if (pair.second != nullptr) {
            pair.second->BeforeFrameUpdate();
        }
    }
    for (auto& pair : m_items_widgets) {
        if (pair.second != nullptr) {
            pair.second->BeforeFrameUpdate();
        }
    }
}

void ItemsManagement::show_cat(Item::CategoryID cat_id, Item::Category_ptr cat) {
    bool is_retired = m_manager->isRetired(cat_id).value();
    bool show = true;
    if (is_retired) {
        if (m_show_retired_cats) {
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.3f, 0.3f, 0.3f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.4f, 0.4f, 0.4f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.5f, 0.5f, 0.5f, 0.7f));
        }
        else
            show = false;
    }
    if (show) {
        if (ImGui::CollapsingHeader(labelize(cat_id, cat->name).c_str())) {
            ImGui::Indent(40.f);
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Actions sur catégorie:");
            ImGui::SameLine();
            if (!is_retired) {
                if (button(labelize(cat_id, "Éditer"), m_ui_state)) {
                    m_cat_widgets[cat_id] = std::make_shared<CategoryWidget>(m_ui_state, cat_id);
                    m_items_widgets[cat_id]->unselectedItem();
                    m_show_item_widget.close();
                }
                ImGui::SameLine();
                if (button(labelize(cat_id, "Retirer"), m_ui_state, "", ImVec4(0.7f, 0.2f, 0.2f, 0.4f))) {
                    const modal_fct error_fct = [this, cat, cat_id](bool& show, bool&, bool&) {
                        ImGui::Text("Vous êtes sur le point de retirer la catégorie ");
                        Tempo::PushFont(m_ui_state->font_bold);
                        ImGui::Text(cat->name.c_str());
                        Tempo::PopFont();
                        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.f);
                        ImGui::Text("Cette action va cacher la catégorie de la liste et tout objet emprunté\n"
                            "de cette catégorie va être considéré comme rendu.\n"
                            "Elle peut être restaurée à tout moment\n\n"
                            "Pour continuer, veuillez taper le nom de la catégorie dans le champ suivant:");
                        ImGui::InputTextWithHint("##retire_cat", "Nom de la catégorie à retirer", &m_retire);

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
                            m_manager->retireCategory(cat_id);
                            show = false;
                            m_workspace.save("retirer_categorie");
                        }
                    };
                    m_retire = "";
                    ImGui::SetNextWindowSizeConstraints(ImVec2(500, 300), ImVec2(FLT_MAX, FLT_MAX));
                    Modals::getInstance().setModal("Voulez-vous retirer la catégorie ?", error_fct);
                }
            }
            else {
                if (button(labelize(cat_id, "Restaurer"), m_ui_state)) {
                    m_manager->unretireCategory(cat_id);
                    show = false;
                    m_workspace.save("restaurer_categorie");
                }
                ImGui::SameLine();
                if (button(labelize(cat_id, "Supprimer définitivement"), m_ui_state, "", ImVec4(0.7f, 0.2f, 0.2f, 0.4f))) {
                    const modal_fct error_fct = [this, cat, cat_id](bool& show, bool&, bool&) {
                        ImGui::Text("Vous êtes sur le point de supprimer la catégorie ");
                        Tempo::PushFont(m_ui_state->font_bold);
                        ImGui::Text(cat->name.c_str());
                        Tempo::PopFont();
                        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.f);
                        ImGui::Text("Cette action va supprimer la catégorie définitivement.\n"
                            "Tout objet appartenant à cette catégorie va aussi être supprimé.\n\n"
                            "Pour continuer, veuillez taper le nom de la catégorie dans le champ suivant:");
                        ImGui::InputTextWithHint("##delete_cat", "Nom de la catégorie à supprimer", &m_delete);

                        if (m_delete == cat->name) {
                            m_delete_error = "";
                        }
                        else {
                            m_delete_error = "Il faut taper le nom pour continuer";
                        }
                        if (button(labelize(cat_id, "Annuler"), m_ui_state, "")) {
                            show = false;
                        }
                        ImGui::SameLine();
                        if (button(labelize(cat_id, "Supprimer"), m_ui_state, m_delete_error)) {
                            m_manager->deleteRetiredCategory(cat_id);
                            show = false;
                            m_workspace.save("supprimer_categorie");
                        }
                    };
                    m_delete = "";
                    Modals::getInstance().setModal("Voulez-vous supprimer la catégorie ?", error_fct);
                }
            }
            ImGui::Separator();

            if (!m_items_widgets.contains(cat_id)) {
                m_items_widgets[cat_id] = std::make_shared<ItemsListWidget>(m_ui_state, cat_id);
            }
            if (m_items_widgets[cat_id]->getEditMode()) {
                m_show_item_widget.close();
                m_unselect_all = true;
            }
            m_items_widgets[cat_id]->FrameUpdate();
            auto item = m_items_widgets[cat_id]->clickedOnItem();
            if (item != nullptr) {
                for (auto widget : m_items_widgets) {
                    if (widget.first == cat_id)
                        continue;
                    widget.second->unselectedItem();
                }
                m_show_item_widget.setItem(item);
            }
            if (m_unselect_all) {
                m_items_widgets[cat_id]->unselectedItem();
            }
            ImGui::Unindent(40.f);
        }
    }

    if (is_retired) {
        if (m_show_retired_cats) {
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
        }
    }
}

const float proportion = 0.7f;

void ItemsManagement::FrameUpdate() {
    m_manager = m_workspace.getCurrentManager();

    float height = ImGui::GetContentRegionAvail().y;

    ImVec2 size;
    if (m_show_item_widget.isOpen()) {
        size.y = proportion * height;
    }
    ImGui::BeginChild("item_management", size);

    title("Catégories d'objets", m_ui_state);
    ImGui::SameLine();
    if (button("+##new_cat", m_ui_state)) {
        m_new_cat = true;
        m_cat_widgets[-1] = std::make_shared<CategoryWidget>(m_ui_state);
    }
    ImGui::SameLine();
    if (button("Propriétés##edit_props_button", m_ui_state, "", ImVec4(), true)) {
        m_ui_state->active_panel = UIState::PROPERTIES;
    }

    if (m_cat_widgets.contains(-1)) {
        m_cat_widgets[-1]->FrameUpdate();
    }

    for (auto cat_id : m_manager->getAllCategories()) {
        auto cat = m_manager->getCategory(cat_id).value();
        if (m_cat_widgets.contains(cat_id)) {
            m_cat_widgets[cat_id]->FrameUpdate();
        }
        else {
            show_cat(cat_id, cat);
        }
    }
    ImGui::Separator();
    ImGui::Checkbox("Afficher les catégories retirées", &m_show_retired_cats);
    ImGui::EndChild();

    m_show_item_widget.BeforeFrameUpdate();
    m_unselect_all = m_show_item_widget.justClosed();

    if (m_show_item_widget.isOpen()) {
        ImGui::BeginChild("show_item", ImVec2(0, height * (1 - proportion - 0.01f)), true);
        m_show_item_widget.FrameUpdate();
        ImGui::EndChild();
    }
}