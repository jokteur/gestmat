#include "select_item_widget.h"
#include "ui/widgets/misc.h"
#include "ui/imgui_util.h"

SelectItemWidget::SelectItemWidget(UIState_ptr ui_state) : Drawable(ui_state), m_filter(ui_state, "Rechercher") {
    m_manager = m_workspace.getCurrentManager();
    m_id = m_ui_state->imID;
    m_ui_state->imID++;
}
void SelectItemWidget::setCategories() {
    m_categories.clear();
    std::vector<std::string> cat_names;
    for (auto cat_id : m_manager->getAllCategories()) {
        if (m_manager->isRetired(cat_id).value())
            continue;
        auto cat = m_manager->getCategory(cat_id).value();
        cat_names.push_back(cat->name);
        m_categories[cat->name] = cat->id;
    }
    m_cat_combo = std::make_shared<Combo>(m_ui_state, cat_names, 0, true);
}
void SelectItemWidget::BeforeFrameUpdate() {
    m_manager = m_workspace.getCurrentManager();

    if (m_cat_combo == nullptr)
        setCategories();

    if (m_sub_id == -1)
        m_sub_id = m_manager->getId();

    if (m_manager->isChanged(m_sub_id)) {
        setCategories();
    }
}

void SelectItemWidget::show_notes(Item::Item_ptr item) {
    ImGui::Text("Notes sur l'objet:");
    ImGui::Indent(20.f);
    for (auto it = item->notes.rbegin();it != item->notes.rend();it++) {
        Tempo::PushFont(m_ui_state->font_bold);
        timestampToText(it->timestamp);
        Tempo::PopFont();
        ImGui::AlignTextToFramePadding();
        ImGui::Text(it->content.c_str());
        ImGui::Separator();
    }
    ImGui::Unindent(20.f);
}

void SelectItemWidget::show_row(Item::ItemID item_id, Item::Category_ptr cat) {
    auto item = m_manager->getItem(item_id).value();

    bool is_loaned = m_manager->isLoaned(item_id);
    if (!m_include_loaned && m_manager->isLoaned(item_id)) {
        return;
    }
    if (m_avoid_items.contains(item_id))
        return;


    if (!m_filter.PassFilter(m_filter_infos[item_id]))
        return;

    ImGui::TableNextRow();
    int extra_cols = m_include_loaned ? 1 : 0;
    if (m_include_loaned) {
        ImGui::TableSetColumnIndex(0);
        if (is_loaned)
            ImGui::Text("Oui");
        else
            ImGui::Text("Non");
    }
    int j = extra_cols;
    for (auto prop_id : cat->properties) {
        if (cat->properties_hide.contains(prop_id))
            continue;
        ImGui::TableSetColumnIndex(j);
        if (j == extra_cols) {
            ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
            ImGui::Selectable(
                labelize(prop_id, item->property_values[prop_id], item->id).c_str(),
                &m_item_selected_map[item->id],
                selectable_flags);
        }
        else
            ImGui::TextWrapped(item->property_values[prop_id].c_str());
        if (!item->notes.empty() && ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            show_notes(item);
            ImGui::EndTooltip();
        }
        j++;
    }
    for (auto pair : m_item_selected_map) {
        if (pair.second)
            m_item_id = pair.first;
    }
}

void SelectItemWidget::fill_items(Item::Category_ptr cat) {
    m_items_list.clear();
    m_filter_infos.clear();

    Item::PropertyID prop_id = -1;
    int j = 0;
    for (auto prop_id2 : cat->properties) {
        if (cat->properties_hide.contains(prop_id2)) {
            continue;
        }
        if (j == m_sort_col_id) {
            prop_id = prop_id2;
        }
        j++;
    }
    if (prop_id == -1) {
        prop_id = cat->properties[0];
    }
    auto prop = m_manager->getProperty(prop_id).value();

    for (auto item_id : cat->registered_items) {
        if (m_manager->isRetired(item_id).value())
            continue;
        auto item = m_manager->getItem(item_id).value();
        m_items_list[item->property_values[prop_id]].push_back(item_id);
        for (auto prop_id3 : cat->properties) {
            if (cat->properties_hide.contains(prop_id3))
                continue;
            m_filter_infos[item_id].insert(
                core::toLower(item->property_values[prop_id3]));
        }
    }
}

void SelectItemWidget::show_items(Item::CategoryID cat_id) {
    if (cat_id == -1)
        return;


    m_filter.FrameUpdate();


    auto cat = m_manager->getCategory(cat_id).value();
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 2));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);

    ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable
        | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable
        | ImGuiTableFlags_SizingStretchProp;


    int extra_cols = 0;
    if (m_include_loaned)
        extra_cols = 1;
    if (ImGui::BeginTable(
        labelize(m_id, "Loan table", cat->id).c_str(),
        (int)cat->properties.size() + extra_cols, flags
    )) {
        // ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_NoSort);
        if (m_include_loaned)
            ImGui::TableSetupColumn("Emprunté?", ImGuiTableColumnFlags_NoSort);
        int i = extra_cols;
        for (auto prop_id : cat->properties) {
            if (cat->properties_hide.contains(prop_id))
                continue;
            int col_flags = 0;
            if (i == extra_cols) {
                col_flags |= ImGuiTableColumnFlags_DefaultSort;
            }
            auto prop = m_manager->getProperty(prop_id).value();
            ImGui::TableSetupColumn(prop->name.c_str(), col_flags);
            i++;
        }
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs()) {
            if (sorts_specs->SpecsDirty) {
                m_ascending = sorts_specs->Specs->SortDirection == ImGuiSortDirection_Ascending ? true : false;
                m_sort_col_id = sorts_specs->Specs->ColumnIndex - extra_cols;
                m_fill_cols = true;
                sorts_specs->SpecsDirty = false;
            }
        }
        if (m_manager->isChanged(m_sub_id) || m_fill_cols) {
            fill_items(cat);
        }

        if (m_ascending) {
            for (auto pair : m_items_list) {
                auto items = pair.second;
                for (auto item_id : items)
                    show_row(item_id, cat);
            }
        }
        else {
            for (auto it = m_items_list.rbegin();it != m_items_list.rend();it++) {
                auto items = it->second;
                for (auto item_id : items)
                    show_row(item_id, cat);
            }
        }

        ImGui::EndTable();
    }
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor();
}

void SelectItemWidget::FrameUpdate() {
    m_manager = m_workspace.getCurrentManager();

    if (m_item_id != -1) {
        auto item = m_manager->getItem(m_item_id).value();
        auto cat = m_manager->getCategory(item->category).value();
        if (button(labelize(m_item_id, "Enlever"), m_ui_state)) {
            m_destroy = true;
        }
        ImGui::SameLine();

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
        ImGui::Text(cat->name.c_str());
        if (m_manager->isLoaned(m_item_id)) {
            ImGui::SameLine();
            ImGui::Text("(déjà emprunté "); ImGui::SameLine();
            help("L'objet va être rendu puis réemprunté avec la nouvelle personne.");
            ImGui::SameLine();
            ImGui::Text(")");
        }
        return;
    }
    else {
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Choississez une catégorie:");
        ImGui::SameLine();
        float item_height = ImGui::GetTextLineHeightWithSpacing();
        const float spacing = item_height * 6;
        ImGui::SetNextItemWidth(spacing);
        if (m_cat_combo != nullptr) {
            m_cat_combo->FrameUpdate();
            std::string select = m_cat_combo->getValue();
            ImGui::Indent(40.f);
            if (!select.empty()) {
                ImGui::Checkbox(labelize(m_id, "Montrer objets empruntés##show_all").c_str(), &m_include_loaned);
                show_items(m_categories[select]);
            }
            ImGui::Unindent(40.f);
        }
    }
}

SelectItemWidget::~SelectItemWidget() {
    if (m_manager != nullptr) {
        m_manager->giveBackId(m_sub_id);
    }
}