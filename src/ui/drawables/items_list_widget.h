#pragma once

#include <memory>

#include "ui/drawable.h"
#include "ui/widgets/combo.h"

using namespace core;

struct ItemInfos {
    Item::Item_ptr item;
    bool is_new = false;
    Item::ItemID id;
    std::map<Item::PropertyID, std::string> values;
    std::map<Item::PropertyID, std::shared_ptr<Combo>> select;
};

class ItemsListWidget : public Drawable {
private:
    Item::CategoryID m_cat_id;

    Item::Manager_ptr m_manager;
    Item::Category_ptr m_category = nullptr;

    bool m_destroy = false;

    bool m_first_created = true;

    bool m_edit_mode = false;

    bool m_fill_cols = true;
    int m_sort_col_id = 0;
    bool m_ascending = true;

    int m_sub_id;
    long long int m_table_id;

    std::map<Item::PropertyID, Item::Property_ptr> m_properties;

    std::map<std::string, std::vector<ItemInfos>> m_items;
    std::vector<ItemInfos> m_new_items;

    void add_empty_item();

    void reset_select_in_new_items();

    void fill_items(bool default_sort = false);

    void change_id();

    void show_row(ItemInfos& item_info);

    void save();
public:
    ItemsListWidget(UIState_ptr ui_state, Item::CategoryID cat_id = -1);
    void FrameUpdate() override;
    void BeforeFrameUpdate() override;

    ~ItemsListWidget() { m_workspace.getCurrentManager()->giveBackId(m_sub_id); }
};