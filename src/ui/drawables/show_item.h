#pragma once

#include <memory>

#include "ui/drawable.h"
#include "ui/widgets/combo.h"

using namespace core;

class ShowItem : public Drawable {
private:
    Item::Item_ptr m_item = nullptr;
    bool m_show = false;

    enum Tab { INFOS, NOTES, HISTORY, ACTIONS };
    Tab m_selected_tab = INFOS;

    Tempo::Listener m_listener;
    std::string m_note;
    bool m_new_note = false;

    Item::Manager_ptr m_manager = nullptr;
    bool m_just_closed = false;

    void person_widget(Item::Person_ptr person);

    void show_infos();
    void show_notes();
    void show_history();
    void show_actions();

    void delete_item();
    void retire_item();
    void unretire_item();
public:
    ShowItem(UIState_ptr ui_state);
    ~ShowItem();

    void setItem(Item::Item_ptr item);

    void FrameUpdate() override;
    void BeforeFrameUpdate() override;

    bool justClosed();
    void close() { m_show = false; m_item = nullptr; }
    bool isOpen();
};