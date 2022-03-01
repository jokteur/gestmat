#include "items_stats.h"
#include "ui/widgets/misc.h"
#include "implot.h"
#include "implot_internal.h"
#include "ui/imgui_util.h"
#include <cmath>


template <typename T>
int BinarySearch(const T* arr, int l, int r, T x) {
    if (r >= l) {
        int mid = l + (r - l) / 2;
        if (arr[mid] == x)
            return mid;
        if (arr[mid] > x)
            return BinarySearch(arr, l, mid - 1, x);
        return BinarySearch(arr, mid + 1, r, x);
    }
    return -1;
}

ItemsStats::ItemsStats(UIState_ptr ui_state) : Drawable(ui_state) {
    m_listener.filter = "change_manager";
    m_listener.callback = [this](Tempo::Event_ptr event) {
        m_manager = m_workspace.getCurrentManager();
        m_sub_id = m_manager->getId();
        m_update_widget = true;
    };
    Tempo::EventQueue::getInstance().subscribe(&m_listener);
}
ItemsStats::~ItemsStats() {
    Tempo::EventQueue::getInstance().unsubscribe(&m_listener);
}

void ItemsStats::reset() {
    // Reset variables
    m_mean_days = 0.f;
    m_max_days = 0;
    m_num_loans = 0;
    m_never_loaned.clear();
    m_ordered.clear();
}

void ItemsStats::set_properties_widget() {
    if (m_manager != nullptr) {
        auto cat_res = m_manager->getCategory(m_cat_id);
        if (!cat_res.has_value()) {
            m_combo_prop = nullptr;
            return;
        }
        auto cat = cat_res.value();
        std::vector<std::string> names;
        for (auto prop_id : cat->properties) {
            auto prop = m_manager->getProperty(prop_id).value();
            names.push_back(prop->name);
            m_properties[prop->name] = prop_id;
        }
        m_combo_prop = std::make_shared<Combo>(m_ui_state, names, "");
    }
}

void ItemsStats::timeline_widget() {
    if (m_timeline_stats.empty())
        return;

    static const char* ilabels[] = { "Emprunt", "Retour" };

    std::vector<double> dates;
    std::vector<Stat> counters;
    for (auto pair : m_timeline_stats) {
        dates.push_back((double)pair.first + 3600);
        counters.push_back(pair.second);
    }

    float height = ImGui::GetTextLineHeight() * 7;
    if (ImPlot::BeginPlot("##timeline", ImVec2(-1, height), ImPlotFlags_NoMouseText)) {
        ImPlot::SetupAxes(NULL, NULL,
            ImPlotAxisFlags_Time | ImPlotAxisFlags_Lock,
            ImPlotAxisFlags_RangeFit | ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoMenus);

        int day_length = 84600;
        ImPlot::SetupAxesLimits((double)toTimestamp(m_timeline_begin) - day_length, (double)getTimestamp() + day_length, 0, m_timeline_max_y);
        ImPlot::SetupAxisFormat(ImAxis_Y1, "%.0f");

        ImPlot::SetupLegend(ImPlotLocation_NorthWest, ImPlotLegendFlags_NoButtons);

        auto plot = ImPlot::GetCurrentPlot();
        auto gl_mouse = ImGui::GetMousePos();
        auto tl = plot->CanvasRect.GetTL();
        auto br = plot->CanvasRect.GetBR();

        if (gl_mouse.x > tl.x && gl_mouse.x < br.x
            && gl_mouse.y > tl.y && gl_mouse.y < br.y) {
            ImPlotPoint mouse = ImPlot::GetPlotMousePos();
            ImGui::BeginTooltip();
            auto datetime = getDatetime((long long)mouse.x);
            auto date = Date(
                (uint8_t)datetime.day,
                (uint8_t)datetime.month,
                (uint16_t)datetime.year);
            ImGui::Text(("Date:" + date.format("%d/%m/%Y")).c_str());
            ImGui::EndTooltip();
        }
        // begin plot item
        if (ImPlot::BeginItem("Emprunté")) {
            // override legend icon color
            // fit data if requested
            if (ImPlot::FitThisFrame()) {
                for (int i = 0; i < dates.size(); ++i) {
                    double sum = counters[i].loans + counters[i].returned;
                    ImPlot::FitPoint(ImPlotPoint(dates[i], 0));
                    ImPlot::FitPoint(ImPlotPoint(dates[i], sum));
                }
            }
            // get ImGui window DrawList
            ImDrawList* draw_list = ImPlot::GetPlotDrawList();
            double half_width = 0.9;
            const double day = 3600 * 24;

            ImVec4 red = ImVec4(116.f / 255.f, 145.f / 255.f, 192.f / 255.f, 1.f);
            // render data
            for (int i = 0; i < dates.size(); ++i) {
                ImVec2 bottom = ImPlot::PlotToPixels(dates[i] - half_width * day / 2, 0);
                ImVec2 top = ImPlot::PlotToPixels(dates[i] + half_width * day / 2, counters[i].loans);
                ImU32 color = ImGui::GetColorU32(red);
                draw_list->AddRectFilled(top, bottom, color);
            }
        }

        if (ImPlot::BeginItem("Rendu")) {
            // override legend icon color
            // fit data if requested
            if (ImPlot::FitThisFrame()) {
                for (int i = 0; i < dates.size(); ++i) {
                    double sum = counters[i].loans + counters[i].returned;
                    ImPlot::FitPoint(ImPlotPoint(dates[i], 0));
                    ImPlot::FitPoint(ImPlotPoint(dates[i], sum));
                }
            }

            // get ImGui window DrawList
            ImDrawList* draw_list = ImPlot::GetPlotDrawList();
            double half_width = 0.9;
            const double day = 3600 * 24;

            ImVec4 green = ImVec4(226.f / 255.f, 151.f / 255.f, 41.f / 255.f, 1.f);
            // render data
            for (int i = 0; i < dates.size(); ++i) {
                double sum = counters[i].loans + counters[i].returned;
                ImVec2 bottom = ImPlot::PlotToPixels(dates[i] - half_width * day / 2, counters[i].loans);
                ImVec2 top = ImPlot::PlotToPixels(dates[i] + half_width * day / 2, sum);
                ImU32 color = ImGui::GetColorU32(green);
                draw_list->AddRectFilled(top, bottom, color);
            }


            // end plot item
            ImPlot::EndItem();
        }

        ImPlot::DragRect(0, &m_rect.X.Min, &m_rect.Y.Min, &m_rect.X.Max, &m_rect.Y.Max, ImVec4(1.f, 0.05f, 1.f, 0.6f),
            ImPlotDragToolFlags_NoFit);

        // end plot item

        ImPlot::EndPlot();
    }
}


void ItemsStats::update_widget() {
    if (m_manager != nullptr && m_update_widget) {
        m_update_widget = false;
        m_timeline_begin = getCurrentDate();
        for (auto loan_id : m_manager->getAllLoans()) {
            auto loan = m_manager->getLoan(loan_id).value();
            if (getDifference(m_timeline_begin, loan->date) < 0)
                m_timeline_begin = loan->date;

            // Timeline
            if (loan->date_back.isValid()) {
                auto end = toTimestamp(loan->date_back);
                m_timeline_stats[end].addReturned();
            }
            auto start = toTimestamp(loan->date);
            m_timeline_stats[start].addLoan();
        }
        m_timeline_max_y = 0;
        for (auto pair : m_timeline_stats) {
            int sum = pair.second.loans + pair.second.returned;
            if (sum > m_timeline_max_y)
                m_timeline_max_y = sum;
        }

        auto cats = m_manager->getAllCategories();
        m_categories.clear();
        std::vector<std::string> selections;
        for (auto cat_id : cats) {
            auto cat = m_manager->getCategory(cat_id).value();
            m_categories[cat->name] = cat_id;
            selections.push_back(cat->name);
        }
        m_combo = std::make_shared<Combo>(m_ui_state, selections, "", true);
        set_properties_widget();

        double start = (double)toTimestamp(m_timeline_begin);
        double end = (double)getTimestamp();

        m_rect = ImPlotRect(start - 42300, end + 42300, -10, 1e6);
    }
}
void ItemsStats::calculate() {
    m_manager = m_workspace.getCurrentManager();
    if (m_manager == nullptr)
        return;

    if (m_cat_id < 0)
        return;

    int num_days = getDifference(m_start, m_end);
    if (num_days < 0) {
        auto tmp = m_start;
        m_start = m_end;
        m_end = tmp;
    }
    auto cat_res = m_manager->getCategory(m_cat_id);
    if (!cat_res.has_value())
        return;
    auto cat = cat_res.value();

    bool find = false;
    for (auto col : cat->properties) {
        if (m_col_id == col) {
            find = true;
            break;
        }
    }
    if (!find)
        return;

    reset();

    std::set<Item::ItemID> items;

    auto today = getCurrentDate();
    std::map<std::string, int> temp_map;
    for (auto loan_id : m_manager->getAllLoans()) {
        auto loan = m_manager->getLoan(loan_id).value();
        auto item_res = m_manager->getItem(loan->item);
        if (!item_res.has_value())
            continue;
        auto item = item_res.value();

        if (item->category != m_cat_id)
            continue;


        auto loan_start = loan->date;
        auto loan_end = loan->date_back;
        if (!loan->date_back.isValid()) {
            loan_end = today;
        }

        int diff = getDifference(loan_end, m_end);
        if (diff < 0)
            continue;
        diff = getDifference(m_start, loan_start);
        if (diff < 0)
            continue;

        std::string col = item->property_values[m_col_id];
        int duration = getDifference(loan_start, loan_end);
        if (duration > m_max_days)
            m_max_days = duration;

        m_mean_days += (float)duration;
        m_num_loans++;
        if (!temp_map.contains(col)) {
            temp_map[col] = 1;
        }
        else {
            temp_map[col]++;
        }
        items.insert(item->id);
    }
    for (auto pair : temp_map) {
        std::string str = pair.first;
        if (str == "")
            str = "pas de données";
        m_ordered[pair.second].push_back(str);
    }
    m_mean_days /= (float)m_num_loans;
    for (auto item_id : cat->registered_items) {
        if (!items.contains(item_id))
            m_never_loaned.insert(item_id);
    }
    m_fill_cols = true;
}

void ItemsStats::show_general() {
    ImGui::Indent(40.f);
    Tempo::PushFont(m_ui_state->font_bold);
    ImGui::Text("Nombre observé:");
    Tempo::PopFont();
    ImGui::SameLine();
    ImGui::Text(std::to_string(m_num_loans).c_str());

    Tempo::PushFont(m_ui_state->font_bold);
    ImGui::Text("Temps moyen (jours)");
    Tempo::PopFont();
    ImGui::SameLine();
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << m_mean_days;
    ImGui::Text(ss.str().c_str());

    Tempo::PushFont(m_ui_state->font_bold);
    ImGui::Text("Temps maximal (jours)");
    Tempo::PopFont();
    ImGui::SameLine();
    ImGui::Text(std::to_string(m_max_days).c_str());
    ImGui::Unindent(40.f);
}

void ItemsStats::show_frequencies() {
    std::vector<const char*> y_ticks_label;
    static const char* xgroups[] = { "Nombre d'utilisations" };
    std::vector<double> positions;
    std::vector<double> x_ticks;
    std::vector<int> data;

    int i = 0;
    int max_num = 0;
    for (auto& pair : m_ordered) {
        for (auto& str : pair.second) {
            if (pair.first > max_num)
                max_num = pair.first;
            y_ticks_label.push_back(str.c_str());
            positions.push_back(i);
            data.push_back(pair.first);
            i++;
        }
    }

    const int MAX_TICKS = 15;
    for (i = 0;i < max_num;) {
        x_ticks.push_back(i);
        i += (int)ceil((double)max_num / (double)MAX_TICKS);
    }
    x_ticks.push_back(max_num);

    // float height = ImGui::GetStyle().FramePadding.y * data.size() * 5;
    if (ImPlot::BeginPlot("##bar_group", ImVec2(-1, -1))) {
        auto prop = m_manager->getProperty(m_col_id).value();

        ImPlot::SetupAxes("Nombre d'emprunts", prop->name.c_str(), ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
        ImPlot::SetupAxisTicks(ImAxis_Y1, &positions[0], (int)data.size(), &y_ticks_label[0]);
        ImPlot::SetupAxisTicks(ImAxis_X1, &x_ticks[0], (int)x_ticks.size());

        const double height_percent = 2. / 3.;
        ImDrawList* draw_list = ImPlot::GetPlotDrawList();

        if (ImPlot::IsPlotHovered()) {
            ImPlotPoint mouse = ImPlot::GetPlotMousePos();
            // std::cout << mouse.x << " " << mouse.y << std::endl;
            mouse.y = ImPlot::RoundTo(mouse.y, 0);
            float  tool_t = ImPlot::PlotToPixels(mouse.x, mouse.y - 0.5).y;
            float  tool_b = ImPlot::PlotToPixels(mouse.x, mouse.y + 0.5).y;
            float  tool_l = ImPlot::GetPlotPos().x;
            float  tool_r = tool_l + ImPlot::GetPlotSize().x;
            ImPlot::PushPlotClipRect();
            draw_list->AddRectFilled(ImVec2(tool_l, tool_t), ImVec2(tool_r, tool_b), IM_COL32(128, 128, 128, 64));
            ImPlot::PopPlotClipRect();
            // // find mouse location index
            int idx = BinarySearch(&positions[0], 0, (int)positions.size() - 1, mouse.y);
            // // // render tool tip (won't be affected by plot clip rect)
            if (idx != -1) {
                ImGui::BeginTooltip();
                // ImPlot::FormatDate(ImPlotTime::FromDouble(xs[idx]), buff, 32, ImPlotDateFmt_DayMoYr, ImPlot::GetStyle().UseISO8601);
                boldAndNormal(prop->name, y_ticks_label[idx], m_ui_state);
                boldAndNormal("Nombre d'emprunts", std::to_string(data[idx]), m_ui_state);
                ImGui::EndTooltip();
            }
        }
        ImPlot::PlotBarGroupsH(xgroups,
            &data[0],
            1,
            (int)data.size(),
            height_percent);
        ImPlot::EndPlot();
    }
}

void ItemsStats::show_row(Item::ItemID item_id, Item::Category_ptr cat) {
    auto item = m_manager->getItem(item_id).value();
    ImGui::TableNextRow();
    int j = 0;
    for (auto prop_id : cat->properties) {
        if (cat->properties_hide.contains(prop_id))
            continue;
        ImGui::TableSetColumnIndex(j);
        ImGui::TextWrapped(item->property_values[prop_id].c_str());
        j++;
    }
}

void ItemsStats::fill_items() {
    m_items_list.clear();
    auto cat = m_manager->getCategory(m_cat_id).value();

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

    for (auto item_id : m_never_loaned) {
        if (m_manager->isRetired(item_id).value())
            continue;
        auto item = m_manager->getItem(item_id).value();
        m_items_list[item->property_values[prop_id]].push_back(item_id);
    }
}

void ItemsStats::show_never_loaned() {
    auto cat = m_manager->getCategory(m_cat_id).value();
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 2));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);

    ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable
        | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable
        | ImGuiTableFlags_ScrollY
        | ImGuiTableFlags_SizingStretchProp;



    if (ImGui::BeginTable(
        labelize(m_cat_id, "Never Loaned").c_str(),
        (int)cat->properties.size(), flags, ImVec2(0, -1)
    )) {
        int i = 0;
        for (auto prop_id : cat->properties) {
            int col_flags = 0;
            if (i == 0) {
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
                m_sort_col_id = sorts_specs->Specs->ColumnIndex;
                m_fill_cols = true;
                sorts_specs->SpecsDirty = false;
            }
        }
        if (m_manager->isChanged(m_sub_id) || m_fill_cols) {
            fill_items();
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

void ItemsStats::FrameUpdate() {
    m_manager = m_workspace.getCurrentManager();
    auto color = ImPlot::GetStyleColorVec4(ImPlotCol_AxisBg);
    ImPlot::PushStyleColor(ImPlotCol_AxisBgHovered, color);
    ImPlot::PushStyleColor(ImPlotCol_AxisBgActive, color);

    // Check if rect has been changed for date selection
    if (m_rect.X.Min != m_prev_LR.x || m_rect.X.Max != m_prev_LR.y) {
        m_recalculate = true;
        m_prev_LR = ImVec2((float)m_rect.X.Min, (float)m_rect.X.Max);

        long long start, end;
        if (m_prev_LR.x > m_prev_LR.y) {
            start = (long long)m_prev_LR.y;
            end = (long long)m_prev_LR.x;
        }
        else {
            start = (long long)m_prev_LR.x;
            end = (long long)m_prev_LR.y;
        }
        auto left_dt = getDatetime(start);
        auto right_dt = getDatetime(end);
        m_start = Date((uint8_t)left_dt.day, (uint8_t)left_dt.month, (uint16_t)left_dt.year);
        m_end = Date((uint8_t)right_dt.day, (uint8_t)right_dt.month, (uint16_t)right_dt.year);
    }

    if (m_manager == nullptr)
        return;
    if (m_sub_id < 0) {
        m_sub_id = m_manager->getId();
    }
    if (m_manager->isChanged(m_sub_id) || m_update_widget) {
        update_widget();
        reset();
        calculate();
    }
    else if (m_recalculate) {
        calculate();
        m_recalculate = false;
    }

    float item_height = ImGui::GetTextLineHeightWithSpacing();
    const float spacing = item_height * 6;
    if (m_combo != nullptr) {
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Catégorie:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(spacing);
        m_combo->FrameUpdate();

        if (m_combo->getValue() != m_cat_name) {
            m_cat_name = m_combo->getValue();
            m_cat_id = m_categories[m_cat_name];
            set_properties_widget();
            m_recalculate = true;
        }
        if (m_combo_prop != nullptr) {
            ImGui::SameLine();
            ImGui::Text("Choisir une colonne:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(spacing);
            m_combo_prop->FrameUpdate();

            if (m_combo_prop->getValue() != m_col_name) {
                m_col_name = m_combo_prop->getValue();
                m_col_id = m_properties[m_col_name];
                m_recalculate = true;
            }
        }
    }

    timeline_widget();

    if (m_max_days > 0.f) {
        ImGui::BeginTabBar("show_stat_bar");
        if (ImGui::BeginTabItem("Général")) {
            m_selected_tab = GENERAL;
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Fréquences des emprunts")) {
            m_selected_tab = PLOT;
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Jamais empruntés")) {
            m_selected_tab = NEVER_LOANED;
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();

        switch (m_selected_tab) {
        case GENERAL:
            show_general();
            break;
        case PLOT:
            show_frequencies();
            break;
        case NEVER_LOANED:
            show_never_loaned();
            break;
        }
    }
    else {
        if (m_cat_id < 0) {
            ImGui::Text("Aucunne donnée à afficher. Veuillez sélectionner une catégorie.");
        }
        else {
            ImGui::Text("Aucune donnée à afficher. Veuillez sélectionner un intervalle de temps (rectangle rose)"
                " avec au moins 1 objet rendu.");
        }
    }
    ImPlot::PopStyleColor(2);
}