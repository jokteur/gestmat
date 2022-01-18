#include <iostream>
#include <tempo.h>

#include "ui/main_window.h"
#include "python/py_api.h"
#include "core/item_manager.h"
#include "core/workspace.h"
#include "core/compress.h"


using namespace core;
int main() {
    PyAPI::Handler::getInstance();

    Tempo::Config config{
        .app_name = "Gestmat",
        .app_title = "Gestion matériel",
    };

    Item::Workspace& workspace = Item::Workspace::getInstance();
    // workspace.save("save1");

    workspace.loadDir();

    auto mnger = workspace.getCurrentManager();
    std::cout << "Size: " << mnger->getAllLoans().size() << std::endl;


    MainApp* app = new MainApp();
    Tempo::Run(app, config);

    return 0;
}