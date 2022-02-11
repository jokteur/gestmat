#pragma once

#include "ui/drawable.h"
#include <memory>

#include "category_widget.h"
#include "items_list_widget.h"
#include "properties_widget.h"
#include "show_item.h"

class ItemsManagement : public Drawable {
private:
    bool m_new_cat = false;
    bool m_show_retired_cats = false;
    Item::Manager_ptr m_manager = nullptr;

    Tempo::Listener m_listener;

    std::string m_retire;
    std::string m_retire_error;
    std::string m_delete;
    std::string m_delete_error;

    ShowItem m_show_item_widget;
    bool m_unselect_all = false;

    std::map<Item::CategoryID, std::shared_ptr<CategoryWidget>> m_cat_widgets;
    std::map<Item::CategoryID, std::shared_ptr<ItemsListWidget>> m_items_widgets;

    std::shared_ptr<CategoryWidget> m_new_cat_widget;

    void show_cat(Item::CategoryID cat_id, Item::Category_ptr cat);

public:
    ItemsManagement(UIState_ptr ui_state);
    ~ItemsManagement();
    void FrameUpdate() override;
    void BeforeFrameUpdate() override;
};