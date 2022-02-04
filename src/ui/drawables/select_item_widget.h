#pragma once

#include "ui/drawable.h"
#include <memory>
#include <map>

#include "core/util.h"
#include "ui/widgets/combo.h"

using namespace core;


class SelectItemWidget : public Drawable {
private:
    bool m_destroy = false;
    Item::Manager_ptr m_manager = nullptr;

    std::shared_ptr<Combo> m_cat_combo = nullptr;
    std::map<std::string, Item::CategoryID> m_categories;
    std::set<Item::ItemID> m_avoid_items;

    Item::CategoryID m_cat_id = -1;
    Item::ItemID m_item_id = -1;
    long long int m_sub_id = -1;
    long long int m_id;

    std::map<std::string, std::vector<Item::ItemID>> m_items_list;
    bool m_ascending = true;
    bool m_fill_cols = true;
    bool m_include_loaned = false;
    int m_sort_col_id = 0;

    void setCategories();

    void show_row(Item::ItemID item_id, Item::Category_ptr cat);
    void fill_items(Item::Category_ptr cat);
    void show_items(Item::CategoryID cat_id);
public:
    SelectItemWidget(UIState_ptr ui_state);
    ~SelectItemWidget();
    void FrameUpdate() override;
    void BeforeFrameUpdate() override;

    void avoidTheseItems(std::set<Item::ItemID> items) { m_avoid_items = items; }
    Item::ItemID getSelectedItem() { return m_item_id; }
    bool destroy_me() { return m_destroy; }
};

typedef std::shared_ptr<SelectItemWidget> select_item_ptr;