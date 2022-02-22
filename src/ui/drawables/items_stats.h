#pragma once

#include "ui/drawable.h"
#include "ui/widgets/combo.h"
#include "core/util.h"

class ItemsStats : public Drawable {
private:
    // std::map<> map;
    Item::Manager_ptr m_manager = nullptr;
    Tempo::Listener m_listener;

    enum Tabs { GENERAL, PLOT, NEVER_LOANED };

    Tabs m_selected_tab = GENERAL;

    float m_mean_days = 0.f;
    int m_max_days = 0;
    int m_num_loans = 0;
    std::map<int, std::vector<std::string>> m_ordered;

    std::set<Item::ItemID> m_never_loaned;

    std::shared_ptr<Combo> m_combo = nullptr;
    std::shared_ptr<Combo> m_combo_prop = nullptr;

    bool m_recalculate = false;
    bool m_update_widget = true;

    std::map<std::string, Item::CategoryID> m_categories;
    std::map<std::string, Item::PropertyID> m_properties;
    Date m_start;
    Date m_timeline_begin;
    Date m_end = getCurrentDate();
    Item::CategoryID m_cat_id;
    std::string m_cat_name;
    std::string m_col_name;
    Item::PropertyID m_col_id = -1;

    long long int m_sub_id = -1;

    void reset();
    void set_properties_widget();
    void update_widget();

    void show_general();
    void show_frequencies();
    void show_never_loaned();

    void calculate();
public:
    ItemsStats(UIState_ptr ui_state);
    ~ItemsStats();

    void FrameUpdate() override;
    void BeforeFrameUpdate() override {}
};