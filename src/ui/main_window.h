#pragma once

#include <tempo.h>

#include "core/workspace.h"

#include "state.h"
#include "drawables/menu_bar.h"
#include "drawables/items_management.h"
#include "drawables/items_loan.h"
#include "drawables/items_state.h"
#include "drawables/nav_bar.h"

// Drawable and widgets

using namespace core;
class MainApp : public Tempo::App {
private:
    bool m_open = false;

    std::string m_open_error;

    Item::Workspace& m_workspace = Item::Workspace::getInstance();

    std::shared_ptr<UIState> ui_state = std::make_shared<UIState>();

    std::vector<Item::File> m_files;

    std::shared_ptr<MenuBar> m_menubar = std::make_shared<MenuBar>(ui_state);
    std::shared_ptr<NavBar> m_navbar = std::make_shared<NavBar>(ui_state);
    std::shared_ptr<ItemsLoans> m_loans = std::make_shared<ItemsLoans>(ui_state);
    std::shared_ptr<ItemsManagement> m_management = std::make_shared<ItemsManagement>(ui_state);
    std::shared_ptr<ItemsState> m_state = std::make_shared<ItemsState>(ui_state);

public:
    virtual ~MainApp() {}

    void InitializationBeforeLoop() override;
    void FrameUpdate() override;
    void BeforeFrameUpdate() override;
};