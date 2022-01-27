#include "items_list_widget.h"

#include "ui/imgui_util.h"
#include "ui/widgets/misc.h"
#include "imgui_stdlib.h"

ItemsListWidget::ItemsListWidget(UIState_ptr ui_state, Item::CategoryID cat_id) : Drawable(ui_state), m_cat_id(cat_id) {
    m_manager = m_workspace.getCurrentManager();

    if (cat_id != -1) {
        m_category = m_manager->getCategory(cat_id).value();
        m_sub_id = m_manager->getId();
    }
    change_id();
}

void ItemsListWidget::change_id() {
    m_ui_state->imID++;
    m_table_id = m_ui_state->imID;
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

    auto& values = item_info.values;

    for (int column = 0;column < m_category->properties.size();column++) {
        ImGui::TableSetColumnIndex(column);
        auto prop_id = m_category->properties[column];
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
            ImGui::Text(values[prop_id].c_str());
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

    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2());
    ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);

    if (m_edit_mode) {
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2());
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2());
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 2));
    }
    if (ImGui::BeginTable(labelize(m_cat_id, "item_list").c_str(), (int)m_properties.size() + 1, flags)) {
        int i = 0;
        for (auto prop_id : m_category->properties) {
            int col_flags = 0;
            if (i == 0) {
                col_flags |= ImGuiTableColumnFlags_DefaultSort;
            }
            ImGui::TableSetupColumn(
                labelize(m_cat_id, m_properties[prop_id]->name).c_str(), col_flags);
            i++;
        }
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs()) {
            if (sorts_specs->SpecsDirty) {
                m_ascending = sorts_specs->Specs->SortDirection == ImGuiSortDirection_Ascending ? true : false;
                m_sort_col_id = sorts_specs->Specs->ColumnIndex;
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
        ImGui::PopStyleVar(4);
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}
void ItemsListWidget::BeforeFrameUpdate() {

}