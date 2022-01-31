#include "items_list_widget.h"

#include "ui/imgui_util.h"
#include "ui/widgets/misc.h"
#include "ui/widgets/modal.h"

#include "imgui_stdlib.h"

ItemsListWidget::ItemsListWidget(UIState_ptr ui_state, Item::CategoryID cat_id) : Drawable(ui_state), m_cat_id(cat_id) {
    m_manager = m_workspace.getCurrentManager();

    if (cat_id != -1) {
        m_category = m_manager->getCategory(cat_id).value();
        m_sub_id = m_manager->getId();
    }
    change_id();
}

void ItemsListWidget::save() {
    for (auto& pair : m_items) {
        auto& items = pair.second;
        for (auto& item_info : items) {
            item_info.item->property_values = item_info.values;
        }
    }
    for (auto& item_infos : m_new_items) {
        Item::Item item;
        item.category = m_cat_id;
        item.property_values = item_infos.values;
        m_manager->createItem(item);
    }
    m_new_items.clear();
    m_workspace.save("nouvel_objet", m_manager);
}

void ItemsListWidget::change_id() {
    m_ui_state->imID++;
    m_table_id = m_ui_state->imID;
}

void ItemsListWidget::notes(ItemInfos& item_info) {
    ImGui::SetNextItemWidth(300.f);
    if (ImGui::BeginPopupContextItem(0, ImGuiPopupFlags_MouseButtonLeft)) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 8));
        ImVec4 color = ImVec4(0.9f, 0.9f, 0.9f, 0.9f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(color.x, color.y, color.z, color.w));

        auto item = item_info.item;
        std::set<Item::NoteID> to_remove;
        for (auto it = item->notes.rbegin();it != item->notes.rend();it++) {
            Tempo::PushFont(m_ui_state->font_bold);
            timestampToText(it->timestamp);
            Tempo::PopFont();
            ImGui::AlignTextToFramePadding();
            ImGui::Text(it->content.c_str());
            ImGui::SameLine();
            if (button(labelize(it->id, "-"), m_ui_state)) {
                to_remove.insert(it->id);
            }
            ImGui::Separator();
        }
        for (auto it = item->notes.begin();it != item->notes.end();it++) {
            if (to_remove.contains(it->id)) {
                item->notes.erase(it);
                m_workspace.save("enlever_note");
                break;
            }
        }
        if (button("+##new_note_plus", m_ui_state)) {
            m_new_note = true;
            m_note = "";
        }
        if (m_new_note) {
            ImGui::InputTextMultiline("##new_note_input", &m_note);
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
                    item->notes.push_back(note);
                }
                m_note = "";
                m_new_note = false;
                m_workspace.save("ajouter_note");
            }
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
        ImGui::EndPopup();
    }
}

void ItemsListWidget::add_empty_item() {
    ItemInfos item;
    item.is_new = true;
    item.id = m_ui_state->imID;
    m_ui_state->imID++;
    m_new_items.push_back(item);
}

void ItemsListWidget::reset_select_in_new_items() {
    for (auto& item : m_new_items) {
        item.select.clear();
    }
}

void ItemsListWidget::remove_new_item(ItemInfos& item_info) {
    for (auto it = m_new_items.begin();it != m_new_items.end();it++) {
        if (it->id == item_info.id) {
            m_new_items.erase(it);
            return;
        }
    }
}

void ItemsListWidget::delete_item(ItemInfos& item_info) {
    const modal_fct error_fct = [this, &item_info](bool& show, bool&, bool&) {
        ImGui::Text("Vous êtes sur de supprimer un objet.");
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.f);
        ImGui::Text("Cette action va rendre supprimer l'objet et ne pourra plus être restauré.\n");

        if (button(labelize(m_cat_id, "Annuler"), m_ui_state)) {
            show = false;
        }
        ImGui::SameLine();
        if (button(labelize(m_cat_id, "Supprimer"), m_ui_state)) {
            m_manager->deleteRetiredItem(item_info.item->id);
            show = false;
            m_workspace.save("supprimer_objet");
        }
    };
    Modals::getInstance().setModal("Voulez-vous retirer un objet ?", error_fct);
}

void ItemsListWidget::retire_item(ItemInfos& item_info) {
    const modal_fct error_fct = [this, &item_info](bool& show, bool&, bool&) {
        ImGui::Text("Vous êtes sur de retirer un objet.");
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.f);
        ImGui::Text("Cette action va rendre tous les emprunts liés à cet objet, mais il ne va pas être supprimé.\n"
            "Un objet retiré ne peut plus être emprunté. Il est possible de restaurer les objets par la suite.");

        if (button(labelize(m_cat_id, "Annuler"), m_ui_state)) {
            show = false;
        }
        ImGui::SameLine();
        if (button(labelize(m_cat_id, "Retirer"), m_ui_state)) {
            m_manager->retireItem(item_info.item->id);
            show = false;
            m_workspace.save("retirer_objet");
        }
    };
    Modals::getInstance().setModal("Voulez-vous retirer un objet ?", error_fct);
}

void ItemsListWidget::unretire_item(ItemInfos& item_info) {
    m_manager->unretireItem(item_info.item->id);
    m_workspace.save("restaurer_objet");
}

void ItemsListWidget::fill_items(bool default_sort) {
    m_items.clear();
    if (default_sort) {
        m_sort_col_id = 0;
        m_ascending = true;
    }
    if (m_sort_col_id > m_category->properties.size() - 1) {
        m_sort_col_id = 0;
    }
    auto prop_sort_id = m_category->properties[m_sort_col_id];
    for (auto item_id : m_category->registered_items) {
        auto item = m_manager->getItem(item_id).value();

        ItemInfos infos{
            .item = item,
            .is_new = false,
            .id = item->id,
        };
        for (auto prop_id : m_category->properties) {
            auto prop = m_manager->getProperty(prop_id).value();
            if (!item->property_values.contains(prop_id)) {
                item->property_values[prop_id] = "";
            }
            if (!prop->select.empty()) {
                infos.select[prop_id] = std::make_shared<Combo>(m_ui_state, prop->select, item->property_values[prop_id], true);
            }
        }
        infos.values = item->property_values;
        std::string prop_value = item->property_values[prop_sort_id];

        m_items[prop_value.empty() ? " " : prop_value].push_back(infos);
    }
    m_fill_cols = false;
}

void ItemsListWidget::show_row(ItemInfos& item_info) {
    ImGui::TableNextRow();
    bool is_retired = false;
    if (!item_info.is_new) {
        is_retired = m_manager->isRetired(item_info.item->id).value();
    }
    auto retired_bg = ImGui::GetColorU32(ImVec4(0.4f, 0.4f, 0.4f, 0.4f));

    // Actions column
    ImGui::TableSetColumnIndex(0);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3);


    ImGui::Indent(5.f);

    if (!item_info.is_new) {
        button(labelize(item_info.id, "Notes"), m_ui_state);
        notes(item_info);

        ImGui::SameLine();
        button(labelize(item_info.id, "Actions"), m_ui_state);
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Pour retirer / supprimer / restaurer des objets");
            ImGui::EndTooltip();
        }
        if (ImGui::BeginPopupContextItem(0, ImGuiPopupFlags_MouseButtonLeft)) {
            if (is_retired) {
                if (button(labelize(item_info.id, "Restaurer"), m_ui_state)) {
                    unretire_item(item_info);
                }
                ImGui::SameLine();
                if (button(labelize(item_info.id, "Supprimer"), m_ui_state, "", ImVec4(0.8f, 0.1f, 0.1f, 0.7f))) {
                    delete_item(item_info);
                }
            }
            else {
                if (button(labelize(item_info.id, "Retirer"), m_ui_state, "", ImVec4(0.8f, 0.1f, 0.1f, 0.7f))) {
                    retire_item(item_info);
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
            ImGui::EndPopup();
        }
    }
    else {
        if (button(labelize(item_info.id, "Enlever"), m_ui_state)) {
            m_to_remove.push_back(item_info);
        }
    }
    if (is_retired) {
        ImGui::TableSetBgColor(
            ImGuiTableBgTarget_RowBg0 | ImGuiTableBgTarget_RowBg1,
            retired_bg
        );
    }
    ImGui::Unindent(5.f);
    ImGui::PopStyleVar();

    auto& values = item_info.values;

    for (int column = 1;column <= m_category->properties.size();column++) {
        ImGui::TableSetColumnIndex(column);
        if (is_retired) {
            ImGui::TableSetBgColor(
                ImGuiTableBgTarget_RowBg0 | ImGuiTableBgTarget_RowBg1,
                retired_bg
            );
        }
        auto prop_id = m_category->properties[column - 1];
        auto prop = m_manager->getProperty(prop_id).value();

        // For new items
        if (!values.contains(prop_id)) {
            values[prop_id] = "";
        }

        if (m_edit_mode) {
            ImGui::SetNextItemWidth(-FLT_MIN);
            std::string label = labelize(prop_id, "input", item_info.id);
            if (prop->select.empty())
                ImGui::InputText(label.c_str(), &values[prop_id]);
            else {
                // New items may have been created, or a new property with
                // select may have appeared
                if (!item_info.select.contains(prop_id)) {
                    item_info.select[prop_id] = std::make_shared<Combo>(m_ui_state, prop->select, "", true);
                }
                item_info.select[prop_id]->FrameUpdate();
                // Could be a better way than retrieving all infos
                item_info.values[prop_id] = item_info.select[prop_id]->getValue();
            }
        }
        else
            ImGui::TextWrapped(values[prop_id].c_str());
    }
}

void ItemsListWidget::FrameUpdate() {
    if (m_category == nullptr)
        return;

    ImGui::AlignTextToFramePadding();
    Tempo::PushFont(m_ui_state->font_bold);
    ImGui::Text("Objets");
    Tempo::PopFont();
    ImGui::SameLine();
    if (m_edit_mode) {
        if (button(labelize(m_cat_id, "Annuler##edit_mode"), m_ui_state)) {
            m_edit_mode = false;
            m_new_items.clear();
        }
        ImGui::SameLine();
        if (button(labelize(m_cat_id, "Sauvegarder##edit_mode"), m_ui_state)) {
            m_edit_mode = false;
            save();
        }
    }
    else {
        if (button(labelize(m_cat_id, "Mode édition"), m_ui_state)) {
            m_edit_mode = true;
            m_sort_col_id = 0;
            m_fill_cols = true;
        }
    }

    if (m_edit_mode) {
        ImGui::SameLine();
        if (button(labelize(m_cat_id, "+").c_str(), m_ui_state)) {
            add_empty_item();
        }
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Ajouter un nouvel objet");
            ImGui::EndTooltip();
        }
    }

    // It should be in BeforeFrameUpdate, but ItemManagement calls
    // first FrameUpdate after this object creation
    for (auto prop_id : m_category->properties) {
        m_properties[prop_id] = m_manager->getProperty(prop_id).value();
    }

    ImGuiTableFlags flags = ImGuiTableFlags_Borders
        | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable
        | ImGuiTableFlags_SizingStretchProp;

    if (!m_edit_mode) {
        flags |= ImGuiTableFlags_Sortable;
        m_new_items.clear();
    }

    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 2));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 2));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);

    if (m_edit_mode) {
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 2));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0);
    }
    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeight();
    const int num_rows = (int)m_items.size() + (int)m_new_items.size() + 1;

    if (ImGui::BeginTable(
        labelize(m_cat_id, "item_list").c_str(),
        (int)m_properties.size() + 1, flags
        // ImVec2(0.f, TEXT_BASE_HEIGHT * (float)num_rows)
    )) {

        int i = 1;
        ImGui::TableSetupColumn(
            labelize(m_cat_id, "Actions").c_str(),
            ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort,
            35.f);

        for (auto prop_id : m_category->properties) {
            int col_flags = 0;
            if (i == 1) {
                col_flags |= ImGuiTableColumnFlags_DefaultSort;
            }
            std::string name = m_properties[prop_id]->name;
            if (m_category->properties_hide.contains(prop_id)) {
                name += "*";
            }
            ImGui::TableSetupColumn(
                labelize(m_cat_id, name).c_str(), col_flags);
            i++;
        }
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs()) {
            if (sorts_specs->SpecsDirty) {
                m_ascending = sorts_specs->Specs->SortDirection == ImGuiSortDirection_Ascending ? true : false;
                m_sort_col_id = sorts_specs->Specs->ColumnIndex - 1;
                m_fill_cols = true;
                sorts_specs->SpecsDirty = false;
            }
        }

        if (m_edit_mode) {
            for (auto& item_info : m_new_items) {
                show_row(item_info);
            }
        }

        // Rows of items
        if (m_manager->isChanged(m_sub_id)) {
            fill_items(true);
            // The select may have change in between
            reset_select_in_new_items();
        }
        else if (m_fill_cols) {
            fill_items();
        }
        if (m_ascending || m_edit_mode) {
            for (auto& row : m_items) {
                for (auto& item_info : row.second)
                    show_row(item_info);
            }
        }
        else {
            for (auto it = m_items.rbegin();it != m_items.rend();it++) {
                for (auto& item_info : it->second)
                    show_row(item_info);
            }
        }
        ImGui::EndTable();
    }
    if (m_edit_mode) {
        ImGui::PopStyleVar(2);
    }
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor();
    ImGui::Spacing();
}
void ItemsListWidget::BeforeFrameUpdate() {
    m_manager = m_workspace.getCurrentManager();
    for (auto& item_info : m_to_remove) {
        remove_new_item(item_info);
    }
}